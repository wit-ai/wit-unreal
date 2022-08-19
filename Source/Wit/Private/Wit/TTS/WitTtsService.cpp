/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/TTS/WitTtsService.h"
#include "AudioMixerDevice.h"
#include "JsonObjectConverter.h"
#include "Wit/Request/WitRequestBuilder.h"
#include "Wit/Request/WitRequestSubsystem.h"
#include "Wit/Request/WitRequestTypes.h"
#include "TTS/Configuration/TtsConfiguration.h"
#include "Wit/Utilities/WitLog.h"
#include "Wit/Utilities/WitHelperUtilities.h"

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
}

/**
 * Is any Wit.ai request currently in progress?
 *
 * @return true if in progress otherwise false
 */
bool UWitTtsService::IsRequestInProgress() const
{
	const UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();
	const bool bIsRequestInProgress = RequestSubsystem != nullptr && RequestSubsystem->IsRequestInProgress();

	return bIsRequestInProgress;
}

/**
 * Sends a text string to Wit to be converted into speech
 *
 * @param ClipSettings [in] The synthesis settings we want to use
 */
void UWitTtsService::ConvertTextToSpeechWithSettings(const FTtsConfiguration& ClipSettings)
{
	const FString ClipId = FWitHelperUtilities::GetVoiceClipId(ClipSettings);
	
	// Check if we already have this in the memory cache
	
	if (MemoryCache != nullptr) 
	{
		USoundWave* CachedClip = MemoryCache->GetClip(ClipId);

		const bool bIsClipCached = CachedClip != nullptr;
		if (bIsClipCached)
		{
			UE_LOG(LogWit, Verbose, TEXT("ConvertTextToSpeechWithSettings: clip found in memory cache (%s)"), *ClipId);
			
			OnSynthesizeResponse.Broadcast(true, CachedClip);
			return;
		}
	}

	// Check if we already have this in the storage cache

	const bool bShouldUseStorageCache = StorageCache != nullptr && StorageCache->ShouldCache(ClipSettings.StorageCacheLocation);
	
	if (bShouldUseStorageCache)
	{
		TArray<uint8> CachedClipData;
		
		const bool bIsClipCached = StorageCache->RequestClip(ClipId, ClipSettings.StorageCacheLocation, CachedClipData);
		if (bIsClipCached)
		{
			UE_LOG(LogWit, Verbose, TEXT("ConvertTextToSpeechWithSettings: clip found in storage cache (%s)"), *ClipId);

			OnStorageCacheRequestComplete(CachedClipData, ClipSettings);
			return;
		}
	}

	// If not cached then we send off a request to Wit.ai

	const bool bHasConfiguration = Configuration != nullptr && !Configuration->Application.ClientAccessToken.IsEmpty();
	
	if (!bHasConfiguration)
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettings: cannot convert text because no configuration found. Please assign a configuration and access token"));
		return;
	}
	
	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	if (RequestSubsystem == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettings: cannot convert text because request subsystem does not exist"));
		return;
	}

	if (RequestSubsystem->IsRequestInProgress())
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettings: cannot convert text because a request is already in progress"));
		return;
	}

	if (ClipSettings.Voice.IsEmpty())
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettings: cannot convert text because no voice is specified and it is required"));
		return;
	}

	LastRequestedClipSettings = ClipSettings;

	UE_LOG(LogWit, Display, TEXT("ConvertTextToSpeechWithSettings: converting text (%s) with voice (%s)"), *ClipSettings.Text, *ClipSettings.Voice);
	
	// Construct the request with the desired configuration. We use the /synthesize endpoint in Wit.ai. See the Wit.ai documentation for more
	// specifics of the parameters to this endpoint

	FWitRequestConfiguration RequestConfiguration{};

	FWitRequestBuilder::SetRequestConfigurationWithDefaults(RequestConfiguration, EWitRequestEndpoint::Synthesize, Configuration->Application.ClientAccessToken, Configuration->Application.ApiVersion, Configuration->Application.URL);
	FWitRequestBuilder::AddFormatContentType(RequestConfiguration, EWitRequestFormat::Json);
	FWitRequestBuilder::AddFormatAccept(RequestConfiguration, EWitRequestFormat::Wav);

	RequestConfiguration.OnRequestError.BindUObject(this, &UWitTtsService::OnSynthesizeRequestError);
	RequestConfiguration.OnRequestComplete.BindUObject(this, &UWitTtsService::OnSynthesizeRequestComplete);

	RequestSubsystem->BeginStreamRequest(RequestConfiguration);

	// Construct the body parameters. The only required one is "q" which is the text we want to convert. We could use UStructToJsonObject
	// but since most of the arguments are optional it's easier to just set them

	const TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject());
	const bool bIsTextTooLong = ClipSettings.Text.Len() > MaximumTextLengthInRequest;
		
	if (bIsTextTooLong)
	{
		UE_LOG(LogWit, Warning, TEXT("ConvertTextToSpeechWithSettings: text is too long - truncating to %d characters"), MaximumTextLengthInRequest);
		RequestBody->SetStringField("q",ClipSettings.Text.Left(MaximumTextLengthInRequest));
	}
	else
	{
		RequestBody->SetStringField("q",ClipSettings.Text);
	}
	
	RequestBody->SetNumberField("speed",ClipSettings.Speed);
	RequestBody->SetNumberField("pitch",ClipSettings.Pitch);
	RequestBody->SetNumberField("gain",ClipSettings.Gain);
	RequestBody->SetStringField("voice",ClipSettings.Voice);
	
	if (!ClipSettings.Style.IsEmpty())
	{
		RequestBody->SetStringField("style",ClipSettings.Style);
	}
	
	RequestSubsystem->WriteJsonData(RequestBody.ToSharedRef());
	RequestSubsystem->EndStreamRequest();
}

/**
 * Sends a text string to Wit to be converted into speech
 *
 * @param TextToConvert [in] The string we want to convert
 */
void UWitTtsService::ConvertTextToSpeech(const FString& TextToConvert)
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

	FWitRequestBuilder::SetRequestConfigurationWithDefaults(RequestConfiguration, EWitRequestEndpoint::Voices, Configuration->Application.ClientAccessToken, Configuration->Application.ApiVersion, Configuration->Application.URL);
	FWitRequestBuilder::AddFormatContentType(RequestConfiguration, EWitRequestFormat::Json);

	RequestConfiguration.OnRequestError.BindUObject(this, &UWitTtsService::OnVoicesRequestError);
	RequestConfiguration.OnRequestComplete.BindUObject(this, &UWitTtsService::OnVoicesRequestComplete);

	RequestSubsystem->BeginStreamRequest(RequestConfiguration);
	RequestSubsystem->EndStreamRequest();
}

/**
 * Unload a single clip
 *
 * @param ClipId [in] Id of the clip to unload
 */
void UWitTtsService::UnloadClip(const FString& ClipId)
{
	if (MemoryCache == nullptr)
	{
		return;
	}

	MemoryCache->RemoveClip(ClipId);
}

/**
 * Unload all clips
 */
void UWitTtsService::UnloadAllClips()
{
	if (MemoryCache == nullptr)
	{
		return;
	}

	MemoryCache->RemoveAllClips();
}

/**
 * Remove a single clip from the storage cache
 *
 * @param ClipId [in] Id of the clip to delete
 * @param CacheLocation [in] location of the clip
 */
void UWitTtsService::DeleteClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation)
{
	UnloadClip(ClipId);
	
	if (StorageCache == nullptr)
	{
		return;
	}

	StorageCache->RemoveClip(ClipId, CacheLocation);
}

/**
 * Remove all clips from the storage cache
 *
 * @param CacheLocation [in] location of the clip
 */
void UWitTtsService::DeleteAllClips(const ETtsStorageCacheLocation CacheLocation)
{
	UnloadAllClips();
	
	if (StorageCache == nullptr)
	{
		return;
	}

	StorageCache->RemoveAllClips(CacheLocation);
}

/**
 * Called when a storage cache request is successfully completed. The binary data will contain the audio wav for the converted text
 *
 * @param BinaryData [in] the binary data
 * @param ClipSettings [in] the clip settings for the clip
 */
void UWitTtsService::OnStorageCacheRequestComplete(const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings)
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
	
	OnSynthesizeResponse.Broadcast(true, SoundWave);
}

/**
 * Called when a Wit synthesize request is successfully completed. The binary response will contain the audio wav for the converted text
 *
 * @param BinaryResponse [in] the final binary response
 * @param JsonResponse [in] the final Json response
 */
void UWitTtsService::OnSynthesizeRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	UE_LOG(LogWit, Verbose, TEXT("OnSynthesizeRequestComplete - Final response size: %d"), BinaryResponse.Num());

	const FString ClipId = FWitHelperUtilities::GetVoiceClipId(LastRequestedClipSettings);
	USoundWave* SoundWave = CreateSoundWaveAndAddToMemoryCache(ClipId, BinaryResponse, LastRequestedClipSettings);

	// In situations where we can't create a sound wave it generally means that the response we received is incomplete or corrupt in some way
	
	if (SoundWave == nullptr)
	{
		OnSynthesizeRequestError(TEXT("Sound wave creation failed"), TEXT("Creating a sound wave from the response failed"));
		return;
	}
	
	// Add to the storage cache. The storage cache stores raw binary data rather than sound waves

	const bool bShouldUseStorageCache = StorageCache != nullptr && StorageCache->ShouldCache(LastRequestedClipSettings.StorageCacheLocation);
	
	if (bShouldUseStorageCache)
	{
		StorageCache->AddClip(ClipId, BinaryResponse, LastRequestedClipSettings);
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
	
	OnSynthesizeResponse.Broadcast(true, SoundWave);
}

/**
 * Called when a synthesize request errors
 *
 * @param ErrorMessage [in] the error message
 * @param HumanReadableErrorMessage [in] longer human readable error message
 */
void UWitTtsService::OnSynthesizeRequestError(const FString ErrorMessage, const FString HumanReadableErrorMessage)
{
	UE_LOG(LogWit, Warning, TEXT("Wit request failed with error: %s - %s"), *ErrorMessage, *HumanReadableErrorMessage);

	// Calling OnSynthesizeResponse is kept for backwards compatibility if people are already using it but is otherwise replaced by OnWitError
	
	OnSynthesizeResponse.Broadcast(false, nullptr);
	OnSynthesizeError.Broadcast(ErrorMessage, HumanReadableErrorMessage);
}

/**
 * Called when a Wit voices request is successfully completed. The response will contain a list of available voices
 *
 * @param BinaryResponse [in] the final binary response
 * @param JsonResponse [in] the final Json response
 */
void UWitTtsService::OnVoicesRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	UE_LOG(LogWit, Verbose, TEXT("OnVoicesRequestComplete - Final response size: %d"), BinaryResponse.Num());

	const bool bIsConversionError = !FJsonObjectConverter::JsonObjectToUStruct(JsonResponse.ToSharedRef(), &AvailableVoices);
	if (bIsConversionError)
	{
		OnVoicesRequestError(TEXT("Json To UStruct failed"), TEXT("Converting the Json response to a UStruct failed"));
		return;
	}
	
	OnVoicesResponse.Broadcast(true);
}

/**
 * Called when a voices request errors
 *
 * @param ErrorMessage [in] the error message
 * @param HumanReadableErrorMessage [in] longer human readable error message
 */
void UWitTtsService::OnVoicesRequestError(const FString ErrorMessage, const FString HumanReadableErrorMessage)
{
	UE_LOG(LogWit, Warning, TEXT("Wit request failed with error: %s - %s"), *ErrorMessage, *HumanReadableErrorMessage);

	// Calling OnSynthesizeResponse is kept for backwards compatibility if people are already using it but is otherwise replaced by OnWitError
	
	OnVoicesResponse.Broadcast(false);
	OnVoicesError.Broadcast(ErrorMessage, HumanReadableErrorMessage);
}

/**
 * Creates a sound wave from binary data and adds it to the memory cache
 *
 * @param ClipId [in] the clip id
 * @param BinaryData [in] the binary data
 * @param ClipSettings [in] settings used in generating the clip
 */
USoundWave* UWitTtsService::CreateSoundWaveAndAddToMemoryCache(const FString& ClipId, const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings)
{
	USoundWave* SoundWave = FWitHelperUtilities::CreateSoundWaveFromRawData(BinaryData.GetData(), BinaryData.Num());

	if (SoundWave == nullptr)
	{
		return nullptr;
	}
	
	// Add to the memory cache. The memory cache stores sound waves

	if (MemoryCache != nullptr)
	{
		MemoryCache->AddClip(ClipId, SoundWave, ClipSettings);
	}

	return SoundWave;
}

#if WITH_EDITORONLY_DATA

/**
 * Write the captured voice input to a wav file. The output file will be written to the project folder's Saved/BouncedWavFiles folder as
 * Wit/RecordedVoiceInput.wav. This only works with 16-bit samples
 */
void UWitTtsService::WriteRawPCMDataToWavFile(const uint8* RawPCMData, int32 RawPCMDataSize, int32 NumChannels, int32 SampleRate)
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
