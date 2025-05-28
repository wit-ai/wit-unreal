/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/TTS/WitTtsService.h"
#include "AudioMixerDevice.h"
#include "Engine/Engine.h"
#include "JsonObjectConverter.h"
#include "Wit/Request/WitRequestBuilder.h"
#include "Wit/Request/WitRequestSubsystem.h"
#include "Wit/Request/WitRequestTypes.h"
#include "Wit/Socket/WitSocketSubsystem.h"
#include "TTS/Configuration/TtsConfiguration.h"
#include "Wit/Utilities/WitLog.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitTtsSpeechSplitter.h"

#ifdef CPP_PLUGIN
THIRD_PARTY_INCLUDES_START
#ifdef check
#pragma push_macro("check")
#undef check
#endif
#ifdef ensure
#undef ensure
#endif
#ifdef event
#undef event
#endif
#ifdef DCHECK_LE
#undef DCHECK_LE
#endif
#ifdef LOG
#undef LOG
#endif

typedef uint8_t u_int8_t;

#include <folly/executors/InlineExecutor.h>

#include "VoiceSDK/v2/interfaces/IVoiceSdkApi.hpp"
#include "VoiceSDK/v2/interfaces/io/ITextStaticInputProvider.hpp"
#include "VoiceSDK/v2/interfaces/io/ITextStreamInputProvider.hpp"
#include "VoiceSDK/v2/interfaces/request/tts/ITextToSpeechRequestParameter.hpp"

#include "VoiceSDK/v2/impl/parameters/VoiceSdkParameter.hpp"
#include "VoiceSDK/v2/impl/RequestParameterFactory.hpp"
#include "VoiceSDK/v2/impl/InputProviderFactory.hpp"
#include "VoiceSDK/v2/impl/unidirection/VoiceSdkApi.hpp"

using namespace meta::voicesdk::v2;

#pragma pop_macro("check")
THIRD_PARTY_INCLUDES_END
#endif

#if WITH_EDITORONLY_DATA

// Use for debug recording of the voice input

static TUniquePtr<Audio::FAudioRecordingData> TTSRecordingData;

#endif

/**
 * Wit API constructor
 */
UWitTtsService::UWitTtsService()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWitTtsService::BeginPlay()
{
	Super::BeginPlay();

	if (bUseWebSocket)
	{
		UWitSocketSubsystem* SocketSubsystem = GEngine->GetEngineSubsystem<UWitSocketSubsystem>();
		SocketSubsystem->OnSocketStateChange.AddUObject(this, &UWitTtsService::OnSocketStateChange);
		SocketSubsystem->CreateSocket(Configuration->Application.ClientAccessToken);
		SocketSubsystem->OnSocketStreamProgress.AddUObject(this, &UWitTtsService::OnSynthesizeRequestProgress);
		SocketSubsystem->OnSocketStreamComplete.AddUObject(this, &UWitTtsService::OnSocketStreamComplete);
		UE_LOG(LogWit, Display, TEXT("BeginPlay: Connection Started"));
	}
}

/**
 * Called when the component is destroyed
 */
void UWitTtsService::BeginDestroy()
{
	Super::BeginDestroy();

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();
	const bool bIsRequestInProgress = RequestSubsystem != nullptr && RequestSubsystem->IsRequestInProgress();

	if (bIsRequestInProgress)
	{
		RequestSubsystem->CancelRequest();
	}
	if (bUseWebSocket)
	{
		UWitSocketSubsystem* SocketSubsystem = GEngine->GetEngineSubsystem<UWitSocketSubsystem>();
		SocketSubsystem->OnSocketStateChange.Clear();
		SocketSubsystem->OnSocketStreamProgress.Clear();
		SocketSubsystem->OnSocketStreamComplete.Clear();
		SocketSubsystem->CloseSocket();
		UE_LOG(LogWit, Display, TEXT("BeginDestroy: WebSocket Cleanup"));
	}
}

/**
 * Is any Wit.ai request currently in progress?
 *
 * @return true if in progress otherwise false
 */
bool UWitTtsService::IsRequestInProgress() const
{
	bool bIsRequestInProgress;
	if (bUseWebSocket)
	{
		UWitSocketSubsystem* SocketSubsystem = GEngine->GetEngineSubsystem<UWitSocketSubsystem>();
		bIsRequestInProgress = SocketSubsystem != nullptr && SocketSubsystem->IsSynthesizeInProgress();
	}
	else
	{
		const UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();
		bIsRequestInProgress = RequestSubsystem != nullptr && RequestSubsystem->IsRequestInProgress();
	}

	return bIsRequestInProgress;
}

/**
 * Sends a text string to Wit to be converted into speech
 *
 * @param ClipSettings [in] The synthesis settings we want to use
 * @param QueueAudio [in] should audio be placed in a queue
 */
void UWitTtsService::ConvertTextToSpeechWithSettings(const FTtsConfiguration& ClipSettings, const bool bQueueAudio)
{
	SplitSpeech(ClipSettings, bQueueAudio);
	ConvertTextToSpeechWithSettingsInternal(true, bQueueAudio);
}

/**
 * Sends a text string to Wit to be converted into speech
 *
 * @param ClipSettings [in] The synthesis settings we want to use
 * @param QueueAudio [in] should audio be placed in a queue
 */
void UWitTtsService::ConvertTextToSpeechWithSettingsInternal(const bool bNewRequest, const bool bQueueAudio)
{
	UE_LOG(LogWit, Verbose, TEXT("ConvertTextToSpeechWithSettingsInternal: Sending message"));
	if (QueuedSettings.IsEmpty())
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettingsInternal: cached settings is empty"));
		return;
	}

	UWitSocketSubsystem* SocketSubsystem = GEngine->GetEngineSubsystem<UWitSocketSubsystem>();

	if (bUseWebSocket)
	{
		if (AudioType != EWitRequestAudioFormat::Pcm)
		{
			UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettingsInternal: WebSocket is not currently supported for (%s), switching to PCM"),
				*UEnum::GetValueAsString(AudioType));
			AudioType = EWitRequestAudioFormat::Pcm;
		}
		ESocketState SocketStatus = SocketSubsystem->GetSocketState();
		if (SocketStatus == ESocketState::Disconnected)
		{
			UE_LOG(LogWit, Display, TEXT("ConvertTextToSpeechWithSettingsInternal: Socket disconnected, restarting"));
			SocketSubsystem->CreateSocket(Configuration->Application.ClientAccessToken);
		}
		else if (SocketStatus != ESocketState::Authenticated)
		{
			UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettingsInternal: Socket not yet connected, retrying"));
			return;
		}
	}
	FTtsConfiguration& RequestClipSettings = QueuedSettings[0];

	if (bQueueAudio)
	{
		PreviousDataIndex = 0;
	}
	else
	{
		if (bNewRequest)
		{
			SoundWaveProcedural = nullptr;
		}
		else
		{
			PreviousDataIndex = 0;
		}
	}

	const FString ClipId = FWitHelperUtilities::GetVoiceClipId(RequestClipSettings);

	// Check if we already have this in the memory cache

	if (MemoryCacheHandler != nullptr)
	{
		USoundWave* CachedClip = MemoryCacheHandler->GetClip(ClipId);

		const bool bIsClipCached = CachedClip != nullptr;
		if (bIsClipCached && !bUseStreaming)
		{
			UE_LOG(LogWit, Verbose, TEXT("ConvertTextToSpeechWithSettingsInternal: clip found in memory cache (%s)"), *ClipId);
			SoundWaveProcedural = nullptr;

			if (EventHandler != nullptr)
			{
				QueuedSettings.RemoveAt(0);
				EventHandler->OnSynthesizeResponse.Broadcast(true, CachedClip);
				if (!QueuedSettings.IsEmpty())
				{
					ConvertTextToSpeechWithSettingsInternal(false, true);
				}
			}

			return;
		}
	}

	// Check if we already have this in the storage cache

	const bool bShouldUseStorageCache = StorageCacheHandler != nullptr && StorageCacheHandler->ShouldCache(RequestClipSettings.StorageCacheLocation);

	if (bShouldUseStorageCache)
	{
		TArray<uint8> CachedClipData;

		const bool bIsClipCached = StorageCacheHandler->RequestClip(ClipId, RequestClipSettings.StorageCacheLocation, CachedClipData);
		if (bIsClipCached)
		{
			UE_LOG(LogWit, Verbose, TEXT("ConvertTextToSpeechWithSettingsInternal: clip found in storage cache (%s)"), *ClipId);

			OnStorageCacheRequestComplete(CachedClipData, RequestClipSettings);
			return;
		}
	}

	// If not cached then we send off a request to Wit.ai

	const bool bHasConfiguration = Configuration != nullptr && !Configuration->Application.ClientAccessToken.IsEmpty();

	if (!bHasConfiguration)
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettingsInternal: cannot convert text because no configuration found. Please assign a configuration and access token"));
		return;
	}
	if (RequestClipSettings.Voice.IsEmpty())
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettings: cannot convert text because no voice is specified and it is required"));
		return;
	}

	LastRequestedClipSettings = RequestClipSettings;

	
	UE_LOG(
        LogWit,
        Verbose,
        TEXT("ConvertTextToSpeechWithSettingsInternal: converting text (%s) with voice (%s)"),
        *RequestClipSettings.Text,
        *RequestClipSettings.Voice);

#ifdef CPP_PLUGIN
#if PLATFORM_ANDROID
    folly::InlineExecutor InlineExecutor;
    folly::Executor::KeepAlive<> ExecutorToken;
    ExecutorToken = getKeepAliveToken(InlineExecutor);

    const std::shared_ptr<IVoiceSdkApi> VoiceApi = std::make_shared<VoiceSdkApi>(
        nullptr, // TODO: Replace with implementation of IVoiceSdkCallbacListener
        ExecutorToken);

	std::shared_ptr<RequestParameterFactory> ParameterFactory = std::make_shared<RequestParameterFactory>();
	std::shared_ptr<ITextToSpeechRequestParameter> TtsRequestParameter = ParameterFactory->createTextToSpeechRequestParameter();
	TtsRequestParameter->setSpeed(RequestClipSettings.Speed);
	TtsRequestParameter->setPitch(RequestClipSettings.Pitch);
	TtsRequestParameter->setVoice(TCHAR_TO_UTF8(*RequestClipSettings.Voice));
	if (!RequestClipSettings.Style.IsEmpty())
	{
		//TODO: Switch RequestClipSettings.Style from FString to common::VoiceStyle enum and set parameter
	}

	std::shared_ptr<InputProviderFactory> ProviderFactory = std::make_shared<InputProviderFactory>();
	if (bUseStreaming)
	{
		std::shared_ptr<ITextStreamInputProvider> StreamInputProvider = ProviderFactory->createTextStreamInputProvider();
		if (!StreamInputProvider)
		{
			UE_LOG(LogWit, Error, TEXT("ConvertTextToSpeechWithSettings: Streaming not yet supported via CPP_PLUGIN"));
			return;
		}
		StreamInputProvider->writeText(TCHAR_TO_UTF8(*RequestClipSettings.Text));
		StreamInputProvider->writeEndOfStream();
	}
	else
	{
		std::shared_ptr<ITextStaticInputProvider> StaticInputProvider = ProviderFactory->createTextStaticInputProvider();
		StaticInputProvider->setText(TCHAR_TO_UTF8(*RequestClipSettings.Text));
		VoiceApi->activate(
			std::string(TCHAR_TO_UTF8(*ClipId)), std::move(TtsRequestParameter), StaticInputProvider);
	}

#endif
#else

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	if (RequestSubsystem == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettingsInternal: cannot convert text because request subsystem does not exist"));
		return;
	}

	if (RequestSubsystem->IsRequestInProgress())
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettingsInternal: cannot convert text because a request is already in progress"));
		if (!bQueueAudio)
		{
			bStopInProgressRequest = true;
		}
		return;
	}

	UE_LOG(
		LogWit,
		Verbose,
		TEXT("ConvertTextToSpeechWithSettingsInternal: converting text (%s) with voice (%s)"),
		*RequestClipSettings.Text,
		*RequestClipSettings.Voice);

	// Construct the request with the desired configuration. We use the /synthesize endpoint in Wit.ai. See the Wit.ai documentation for more
	// specifics of the parameters to this endpoint

	FWitRequestConfiguration RequestConfiguration{};

	FWitRequestBuilder::SetRequestConfigurationWithDefaults(RequestConfiguration, EWitRequestEndpoint::Synthesize, Configuration->Application.ClientAccessToken,
		Configuration->Application.Advanced.ApiVersion, Configuration->Application.Advanced.URL);
	FWitRequestBuilder::AddFormatContentType(RequestConfiguration, EWitRequestFormat::Json);
	FWitRequestBuilder::AddFormatAccept(RequestConfiguration, AudioType);

	RequestConfiguration.bShouldUseCustomHttpTimeout = Configuration->Application.Advanced.bIsCustomHttpTimeout;
	RequestConfiguration.HttpTimeout = Configuration->Application.Advanced.HttpTimeout;
	RequestConfiguration.bShouldUseChunkedTransfer = bUseStreaming;

	RequestConfiguration.OnRequestError.AddUObject(this, &UWitTtsService::OnSynthesizeRequestError);
	RequestConfiguration.OnRequestComplete.AddUObject(this, &UWitTtsService::OnSynthesizeRequestComplete);
	if (bUseStreaming && AudioType != EWitRequestAudioFormat::Pcm)
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettingsInternal: Audio streaming is not currently supported for (%s)"), *UEnum::GetValueAsString(AudioType));
		bUseStreaming = false;
	}
	if (bUseStreaming)
	{
		RequestConfiguration.OnRequestProgress.AddUObject(this, &UWitTtsService::OnSynthesizeRequestProgress);
	}

	// Construct the body parameters. The only required one is "q" which is the text we want to convert. We could use UStructToJsonObject
	// but since most of the arguments are optional it's easier to just set them

	const TSharedPtr<FJsonObject> RequestBody = MakeShared<FJsonObject>();
	const bool bIsTextTooLong = RequestClipSettings.Text.Len() > MaximumTextLengthInRequest;

	if (bIsTextTooLong)
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettingsInternal: text is too long, the limit is %d characters"), MaximumTextLengthInRequest);
	}

	RequestBody->SetStringField("q", RequestClipSettings.Text);

	RequestBody->SetNumberField("speed", RequestClipSettings.Speed);
	RequestBody->SetNumberField("pitch", RequestClipSettings.Pitch);
	RequestBody->SetNumberField("gain", RequestClipSettings.Gain);
	RequestBody->SetStringField("voice", RequestClipSettings.Voice);

	if (!RequestClipSettings.Style.IsEmpty())
	{
		RequestBody->SetStringField("style", RequestClipSettings.Style);
	}

	if (bUseWebSocket)
	{
		SocketSubsystem->SendJsonData(ERequestType::Synthesize, RequestBody.ToSharedRef());
	}
	else
	{
		RequestSubsystem->BeginStreamRequest(RequestConfiguration);
		RequestSubsystem->WriteJsonData(RequestBody.ToSharedRef());
		RequestSubsystem->EndStreamRequest();
	}

	QueuedSettings.RemoveAt(0);
#endif
}

/**
 * Sends a text string to Wit to be converted into speech
 *
 * @param TextToConvert [in] The string we want to convert
 */
void UWitTtsService::ConvertTextToSpeech(const FString& TextToConvert, const bool bQueueAudio)
{
	const bool bHasVoicePreset = VoicePreset != nullptr;

	if (!bHasVoicePreset)
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeech: no voice preset found. Please assign a voice preset"));
		return;
	}

	FTtsConfiguration ClipSettings(VoicePreset->Synthesize);
	ClipSettings.Text = TextToConvert;

	ConvertTextToSpeechWithSettings(ClipSettings);
}

/**
 * Splits a speech segment into smaller segments
 *
 * @param ClipSettings [in] the string we want to convert to speech
 * @param QueueAudio [in] should audio be placed in a queue
 */
void UWitTtsService::SplitSpeech(const FTtsConfiguration& ClipSettings, const bool bQueueAudio)
{
	if (!bQueueAudio)
	{
		QueuedSettings.Empty();
	}
	if (!FWitTtsSpeechSplitter::NeedsSplit(ClipSettings.Text, MaximumTextLengthInRequest))
	{
		QueuedSettings.Add(ClipSettings);
		return;
	}
	TArray<FString> NewSpeech = FWitTtsSpeechSplitter::SplitSpeech(ClipSettings.Text, MaximumTextLengthInRequest);

	for (FString Text : NewSpeech)
	{
		FTtsConfiguration NewClipSettings;
		NewClipSettings.Gain = ClipSettings.Gain;
		NewClipSettings.Pitch = ClipSettings.Pitch;
		NewClipSettings.Speed = ClipSettings.Speed;
		NewClipSettings.StorageCacheLocation = ClipSettings.StorageCacheLocation;
		NewClipSettings.Style = ClipSettings.Style;
		NewClipSettings.Voice = ClipSettings.Voice;
		NewClipSettings.Text = Text;

		QueuedSettings.Add(NewClipSettings);
	}
}

/**
 * Fetch a list of available voices from Wit
 */
void UWitTtsService::FetchAvailableVoices()
{
	const bool bHasConfiguration = Configuration != nullptr && !Configuration->Application.ClientAccessToken.IsEmpty();

	if (!bHasConfiguration)
	{
		UE_LOG(LogWit, Warning, TEXT("FetchAvailableVoices: cannot fetch available voices because no configuration found. Please assign a configuration and access token"));
		return;
	}

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	if (RequestSubsystem == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("FetchAvailableVoices: cannot fetch available voices because request subsystem does not exist"));
		return;
	}

	if (RequestSubsystem->IsRequestInProgress())
	{
		UE_LOG(LogWit, Warning, TEXT("FetchAvailableVoices: cannot fetch available voicest because a request is already in progress"));
		return;
	}

	UE_LOG(LogWit, Display, TEXT("FetchAvailableVoices: fetching available voices"));

	// Construct the request with the desired configuration. We use the /voices endpoint in Wit.ai. See the Wit.ai documentation for more
	// specifics of the parameters to this endpoint

	FWitRequestConfiguration RequestConfiguration{};

	FWitRequestBuilder::SetRequestConfigurationWithDefaults(RequestConfiguration, EWitRequestEndpoint::GetVoices, Configuration->Application.ClientAccessToken,
		Configuration->Application.Advanced.ApiVersion, Configuration->Application.Advanced.URL);
	FWitRequestBuilder::AddFormatContentType(RequestConfiguration, EWitRequestFormat::Json);

	RequestConfiguration.bShouldUseCustomHttpTimeout = Configuration->Application.Advanced.bIsCustomHttpTimeout;
	RequestConfiguration.HttpTimeout = Configuration->Application.Advanced.HttpTimeout;

	RequestConfiguration.OnRequestError.AddUObject(this, &UWitTtsService::OnVoicesRequestError);
	RequestConfiguration.OnRequestComplete.AddUObject(this, &UWitTtsService::OnVoicesRequestComplete);

	RequestSubsystem->BeginStreamRequest(RequestConfiguration);
	RequestSubsystem->EndStreamRequest();
}


/**
 * Called when the state of a WebSocket connection changes
 */
void UWitTtsService::OnSocketStateChange()
{
	UWitSocketSubsystem* SocketSubsystem = GEngine->GetEngineSubsystem<UWitSocketSubsystem>();
	ESocketState SocketStatus = SocketSubsystem->GetSocketState();
	FString Status = *UEnum::GetValueAsString(SocketStatus);
	UE_LOG(LogWit, Verbose, TEXT("OnSocketStateChange %s"), *Status);

	if (SocketStatus == ESocketState::Authenticated)
	{
		if (bUseWebSocket && !SocketSubsystem->IsSynthesizeInProgress() && !QueuedSettings.IsEmpty())
		{
			const bool bNewRequest = true;
			const bool bQueueAudio = true;
			ConvertTextToSpeechWithSettingsInternal(bNewRequest, bQueueAudio);
		}
	}
	if (SocketStatus == ESocketState::Disconnected && !QueuedSettings.IsEmpty())
	{
		SocketSubsystem->CreateSocket(Configuration->Application.ClientAccessToken);
	}
}

/**
 * Called when a WebSocket stream is complete
 */
void UWitTtsService::OnSocketStreamComplete()
{
	if (bUseWebSocket && !QueuedSettings.IsEmpty())
	{
		const bool bNewRequest = true;
		const bool bQueueAudio = true;
		ConvertTextToSpeechWithSettingsInternal(bNewRequest, bQueueAudio);
	}
}

/**
 * Called when a storage cache request is successfully completed. The binary data will contain the audio wav for the converted text
 *
 * @param BinaryData [in] the binary data
 * @param ClipSettings [in] the clip settings for the clip
 */
void UWitTtsService::OnStorageCacheRequestComplete(const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings) const
{
	UE_LOG(LogWit, Verbose, TEXT("OnStorageCacheRequestComplete - Data size: %d"), BinaryData.Num());

	const FString ClipId = FWitHelperUtilities::GetVoiceClipId(ClipSettings);
	USoundWave* SoundWave = CreateSoundWaveAndAddToMemoryCache(ClipId, BinaryData, ClipSettings);

	// In situations where we can't create a sound wave it generally means that the response we received is incomplete or corrupt in some way

	if (SoundWave == nullptr)
	{
		OnSynthesizeRequestError(TEXT("Sound wave creation failed"), TEXT("Creating a sound wave from the response failed"));
		return;
	}

	if (EventHandler != nullptr)
	{
		EventHandler->OnSynthesizeRawResponseMulticast.Broadcast(BinaryData);
		EventHandler->OnSynthesizeRawResponse.Broadcast(ClipId, BinaryData, ClipSettings);
		EventHandler->OnSynthesizeResponse.Broadcast(true, SoundWave);
	}
}

/**
 * Called when a Wit synthesize request is successfully completed. The binary response will contain the audio wav for the converted text
 *
 * @param BinaryResponse [in] the final binary response
 * @param JsonResponse [in] the final Json response
 */
void UWitTtsService::OnSynthesizeRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	UE_LOG(LogWit, Verbose, TEXT("OnSynthesizeRequestComplete - Final response size: %d"), QueuedSettings.Num());

	const FString ClipId = FWitHelperUtilities::GetVoiceClipId(LastRequestedClipSettings);
	USoundWave* SoundWave = CreateSoundWaveAndAddToMemoryCache(ClipId, BinaryResponse, LastRequestedClipSettings);

	// In situations where we can't create a sound wave it generally means that the response we received is incomplete or corrupt in some way

	if (SoundWave == nullptr)
	{
		OnSynthesizeRequestError(TEXT("Sound wave creation failed"), TEXT("Creating a sound wave from the response failed"));
		return;
	}

	// Add to the storage cache. The storage cache stores raw binary data rather than sound waves

	const bool bShouldUseStorageCache = StorageCacheHandler != nullptr && StorageCacheHandler->ShouldCache(LastRequestedClipSettings.StorageCacheLocation);

	if (bShouldUseStorageCache)
	{
		StorageCacheHandler->AddClip(ClipId, BinaryResponse, LastRequestedClipSettings);
	}

#if WITH_EDITORONLY_DATA

	// Output the wav file to a file for debugging purposes

	if (bIsWavFileOutputEnabled)
	{
		FWaveModInfo WaveInfo;

		WaveInfo.ReadWaveInfo(BinaryResponse.GetData(), BinaryResponse.Num());
		WriteRawPCMDataToWavFile(WaveInfo.SampleDataStart, WaveInfo.SampleDataSize, *WaveInfo.pChannels, *WaveInfo.pSamplesPerSec);
	}

#endif

	if (EventHandler != nullptr && !bStopInProgressRequest)
	{
		EventHandler->OnSynthesizeRawResponseMulticast.Broadcast(BinaryResponse);
		EventHandler->OnSynthesizeRawResponse.Broadcast(ClipId, BinaryResponse, LastRequestedClipSettings);
		if (!SoundWaveProcedural)
		{

			EventHandler->OnSynthesizeResponse.Broadcast(true, SoundWave);
		}
		else
		{
			const uint8* RawData = BinaryResponse.GetData();
			const int32 RawDataSize = BinaryResponse.Num() % 2 == 0 ? BinaryResponse.Num() : BinaryResponse.Num() - 1;
			const bool bShouldCheckSize = true;
			AddProceduralData(RawData, RawDataSize, bShouldCheckSize);
		}
	}

	bStopInProgressRequest = false;

	if (!QueuedSettings.IsEmpty())
	{
		ConvertTextToSpeechWithSettingsInternal(false, true);
	}
}

/** Called when a Wit synthesize request is in progress to process the incremental payload
*
* @param BinaryData [in] the binary data
* @param ClipSettings [in] the clip settings for the clip
*/
void UWitTtsService::OnSynthesizeRequestProgress(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	if (bStopInProgressRequest)
	{
		SoundWaveProcedural = nullptr;
		return;
	}
	const uint8* RawData = BinaryResponse.GetData();
	const int32 RawDataSize = BinaryResponse.Num() % 2 == 0 ? BinaryResponse.Num() : BinaryResponse.Num() - 1;
	const bool bShouldCheckSize = false;
	AddProceduralData(RawData, RawDataSize, bShouldCheckSize);
}

/**
 * Called when a synthesize request errors
 *
 * @param ErrorMessage [in] the error message
 * @param HumanReadableErrorMessage [in] longer human readable error message
 */
void UWitTtsService::OnSynthesizeRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage) const
{
	UE_LOG(LogWit, Warning, TEXT("OnSynthesizeRequestError: %s - %s"), *ErrorMessage, *HumanReadableErrorMessage);

	// Calling OnSynthesizeResponse is kept for backwards compatibility if people are already using it but is otherwise replaced by OnWitError

	if (EventHandler != nullptr)
	{
		EventHandler->OnSynthesizeResponse.Broadcast(false, nullptr);
		EventHandler->OnSynthesizeError.Broadcast(ErrorMessage, HumanReadableErrorMessage);
	}
}

/**
 * Called when a Wit voices request is successfully completed. The response will contain a list of available voices
 *
 * @param BinaryResponse [in] the final binary response
 * @param JsonResponse [in] the final Json response
 */
void UWitTtsService::OnVoicesRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	if (EventHandler == nullptr)
	{
		return;
	}

	UE_LOG(LogWit, Verbose, TEXT("OnVoicesRequestComplete - Final response size: %d"), BinaryResponse.Num());

	const bool bIsConversionError = !FJsonObjectConverter::JsonObjectToUStruct(JsonResponse.ToSharedRef(), &EventHandler->VoicesResponse);
	if (bIsConversionError)
	{
		OnVoicesRequestError(TEXT("Json To UStruct failed"), TEXT("Converting the Json response to a UStruct failed"));
		return;
	}
}

/**
 * Called when a voices request errors
 *
 * @param ErrorMessage [in] the error message
 * @param HumanReadableErrorMessage [in] longer human readable error message
 */
void UWitTtsService::OnVoicesRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage) const
{
	UE_LOG(LogWit, Warning, TEXT("OnVoicesRequestError: %s - %s"), *ErrorMessage, *HumanReadableErrorMessage);
}

/**
 * Creates a sound wave from binary data and adds it to the memory cache
 *
 * @param ClipId [in] the clip id
 * @param BinaryData [in] the binary data
 * @param ClipSettings [in] settings used in generating the clip
 */
USoundWave* UWitTtsService::CreateSoundWaveAndAddToMemoryCache(const FString& ClipId, const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings) const
{
	USoundWave* SoundWave = FWitHelperUtilities::CreateSoundWaveFromRawData(BinaryData.GetData(), BinaryData.Num(), AudioType, false /* bUseStreaming */);

	if (SoundWave == nullptr)
	{
		return nullptr;
	}

	// Add to the memory cache. The memory cache stores sound waves

	if (MemoryCacheHandler != nullptr)
	{
		MemoryCacheHandler->AddClip(ClipId, SoundWave, ClipSettings);
	}

	return SoundWave;
}

/** Adds incremental raw data to the Procedural Sound Wave buffer queue
*
* @param RawData [in] data to be added to buffer queue
* @param RawDataSize [in] size of the data to be added
* @param bShouldCheckSize [in] should the size be checked against min buffer size
*/
void UWitTtsService::AddProceduralData(const uint8* RawData, const int32 RawDataSize, bool bShouldCheckSize)
{
	const int32 MinBufferLength = BytesPerDataSample * DefaultSampleRate * InitialStreamBufferSize;

	if (!SoundWaveProcedural)
	{
		const bool bIsProcedural = true;
		SoundWaveProcedural = Cast<USoundWaveProcedural>(FWitHelperUtilities::CreateSoundWaveFromRawData(
			RawData,
			RawDataSize,
			AudioType,
			bIsProcedural));
		SoundWaveProcedural->bCanProcessAsync = true;
		PreviousDataIndex = 0;
		if (EventHandler)
		{
			EventHandler->OnSynthesizeResponse.Broadcast(true, SoundWaveProcedural);
		}
	}
	SoundWaveProcedural->Duration = float(RawDataSize) / BytesPerDataSample / DefaultSampleRate;
	UE_LOG(LogWit, Verbose, TEXT("AddProceduralData - Duration: %f"), SoundWaveProcedural->Duration);
	if (bShouldCheckSize || RawDataSize >= MinBufferLength)
	{
		const int IncreaseBufferLength = RawDataSize - (bUseWebSocket ? 0 : PreviousDataIndex);
		if (IncreaseBufferLength <= 0)
		{
			if (IncreaseBufferLength < 0) // Warn only if length is negative
			{
				UE_LOG(LogWit, Warning, TEXT("AddProceduralData: Trying to increase Buffer length by a non-positive amount"));
			}
			return;
		}

		BufferQueue.SetNum(IncreaseBufferLength);
		int8* Data = (int8*)&BufferQueue[0];
		for (size_t i = 0; i < IncreaseBufferLength; ++i)
		{
			Data[i] = RawData[i + (bUseWebSocket ? 0 : PreviousDataIndex)];
		}
		SoundWaveProcedural->QueueAudio((const uint8*)Data, IncreaseBufferLength);
		PreviousDataIndex = RawDataSize;
	}
}

#if WITH_EDITORONLY_DATA

/**
 * Write the captured voice input to a wav file. The output file will be written to the project folder's Saved/BouncedWavFiles folder as
 * Wit/RecordedVoiceInput.wav. This only works with 16-bit samples
 */
void UWitTtsService::WriteRawPCMDataToWavFile(const uint8* RawPCMData, const int32 RawPCMDataSize, const int32 NumChannels, const int32 SampleRate)
{
	TTSRecordingData.Reset(new Audio::FAudioRecordingData());
	TTSRecordingData->InputBuffer = Audio::TSampleBuffer<int16>(reinterpret_cast<const int16*>(RawPCMData), RawPCMDataSize / 2, NumChannels, SampleRate);

	const FString WavFileName = TEXT("SynthesisOutput");
	FString WavFilePath = TEXT("Wit");

	TTSRecordingData->Writer.BeginWriteToWavFile(TTSRecordingData->InputBuffer, WavFileName, WavFilePath, []()
		{
			TTSRecordingData.Reset();
		});
}

#endif
