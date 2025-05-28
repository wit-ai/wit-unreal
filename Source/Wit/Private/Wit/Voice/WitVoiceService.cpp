/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Voice/WitVoiceService.h"
#include "Engine/Engine.h"
#include "JsonObjectConverter.h"
#include "Voice/Capture/VoiceCaptureSubsystem.h"
#include "Wit/Request/WitRequestBuilder.h"
#include "Wit/Request/WitRequestSubsystem.h"
#include "Wit/Socket/WitSocketSubsystem.h"
#include "Wit/Utilities/WitLog.h"
#include "AudioMixerDevice.h"
#include "Wit/Utilities/WitHelperUtilities.h"

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
#include "VoiceSDK/v2/interfaces/io/IAudioStreamInputProvider.hpp"
#include "VoiceSDK/v2/interfaces/request/command/ICommandRequestParameter.hpp"
#include "VoiceSDK/v2/interfaces/request/composer/IComposerRequestParameter.hpp"

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

static TUniquePtr<Audio::FAudioRecordingData> RecordingData;
static TArray<uint8> RecordedVoiceInputBuffer;

#endif

/**
 * Constructor
 */
UWitVoiceService::UWitVoiceService()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = true;
}

/**
 * Called when the component starts playing
 */
void UWitVoiceService::BeginPlay()
{
	Super::BeginPlay();

	// We prevent the component ticking until voice activation occurs. This is to prevent multiple components ticking at the same time and
	// interfering with each other and also to prevent unnecessary work

	SetComponentTickEnabled(false);

	if (bUseWebSocket)
	{
		UWitSocketSubsystem* SocketSubsystem = GEngine->GetEngineSubsystem<UWitSocketSubsystem>();
		SocketSubsystem->OnSocketStateChange.AddUObject(this, &UWitVoiceService::OnSocketStateChange);
		SocketSubsystem->CreateSocket(Configuration->Application.ClientAccessToken);
		SocketSubsystem->OnSocketStreamProgress.AddUObject(this, &UWitVoiceService::OnSpeechRequestProgress);
	}

	UE_LOG(LogWit, Display, TEXT("BeginPlay: Connection Started"));
}

/**
 * Called when the component is destroyed
 */
void UWitVoiceService::BeginDestroy()
{
	UVoiceCaptureSubsystem* VoiceCaptureSubsystem = GEngine->GetEngineSubsystem<UVoiceCaptureSubsystem>();

	const bool bIsVoiceCaptureAvailable = VoiceCaptureSubsystem != nullptr && VoiceCaptureSubsystem->IsCaptureAvailable();
	
	if (bIsVoiceCaptureAvailable)
	{
		VoiceCaptureSubsystem->Shutdown();
	}

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();
	const bool bIsRequestInProgress = RequestSubsystem != nullptr && RequestSubsystem->IsRequestInProgress();

	if (bUseWebSocket)
	{
		UWitSocketSubsystem* SocketSubsystem = GEngine->GetEngineSubsystem<UWitSocketSubsystem>();
		SocketSubsystem->OnSocketStateChange.Clear();
		SocketSubsystem->OnSocketStreamProgress.Clear();
		SocketSubsystem->CloseSocket();
	}
	
	if (bIsRequestInProgress)
	{
		RequestSubsystem->CancelRequest();
	}

	bIsVoiceInputActive = false;
	bIsVoiceStreamingActive = false;

	Super::BeginDestroy();
}

/**
 * Updates the component every frame
 *
 * @param DeltaTime [in] the time in seconds that has pass since the last frame
 * @param TickType [in] the kind of tick this is, for example, are we paused, or 'simulating' in the editor
 * @param ThisTickFunction [in] internal tick function struct that caused this to run
 */
void UWitVoiceService::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsVoiceInputActive)
	{
		return;
	}
	
	if (Configuration == nullptr)
	{
		return;
	}

	UVoiceCaptureSubsystem* VoiceCaptureSubsystem = GEngine->GetEngineSubsystem<UVoiceCaptureSubsystem>();
	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();	
	const bool bIsRequiredSubsystems = VoiceCaptureSubsystem != nullptr && RequestSubsystem != nullptr;
	
	if (!bIsRequiredSubsystems || !VoiceCaptureSubsystem->IsCapturing())
	{
		return;
	}

	const bool bIsVoiceDataAvailable = VoiceCaptureSubsystem->Read();
	const float CurrentVoiceAmplitude =  VoiceCaptureSubsystem->GetCurrentAmplitude();

	LastActivateTime += DeltaTime;

	// If we are not already streaming the voice data to Wit.ai then check to see if we've breached the threshold and should start streaming
	
	if (!bIsVoiceStreamingActive)
	{
		const bool bIsWakeThresholdReached = bIsVoiceDataAvailable && CurrentVoiceAmplitude > Configuration->Voice.WakeMinimumVolume;
		const bool bIsWakeTimeReached = LastActivateTime >= Configuration->Voice.WakeMinimumTime;
			
		if (!bIsWakeThresholdReached || !bIsWakeTimeReached)
		{
			return;
		}
	
		BeginStreamRequest();
	}
	
	LastWakeTime += DeltaTime;

	// Check for and read any new voice data that is available. Voice data may or may not be available depending on
	// whether the user breaks a pre-defined volume threshold
	
	if (bIsVoiceDataAvailable && RequestSubsystem->IsRequestInProgress())
	{
#if WITH_EDITORONLY_DATA
		
		if (Configuration->Voice.bIsWavFileRecordingEnabled)
		{
			RecordedVoiceInputBuffer.Append(VoiceCaptureSubsystem->GetVoiceBuffer());
		}

#endif

#ifdef CPP_PLUGIN
#if PLATFORM_ANDROID
		if (!StreamInputProvider)
		{
			UE_LOG(LogWit, Error, TEXT("TickComponent: Not Initialized"));
			return;
		}
		std::unique_ptr<folly::IOBuf> chunk;

		StreamInputProvider->writeBytes(folly::IOBuf::copyBuffer(&VoiceCaptureSubsystem->GetVoiceBuffer(), VoiceCaptureSubsystem->GetVoiceBuffer().Num()));
#endif
#else
		RequestSubsystem->WriteBinaryData(VoiceCaptureSubsystem->GetVoiceBuffer());
#endif
	}

	UWitSocketSubsystem* SocketSubsystem = GEngine->GetEngineSubsystem<UWitSocketSubsystem>();
	if (bIsVoiceDataAvailable && SocketSubsystem->IsConverseInProgress())
	{
		SocketSubsystem->SendBinaryData(VoiceCaptureSubsystem->GetVoiceBuffer());
	}

	// Keep track of whether we are actually receiving suitable voice input. This is used in deciding when to auto deactivate
	// due to no voice input

	const bool bIsAmplitudeAboveMinimumVolume = CurrentVoiceAmplitude > Configuration->Voice.KeepAliveMinimumVolume;
	
	if (bIsVoiceDataAvailable && bIsAmplitudeAboveMinimumVolume)
	{
		LastVoiceTime = 0.0f;
	}
	else
	{
		LastVoiceTime += DeltaTime;
	}

	// Check for auto deactivation. This can happen in two cases
	// 1. If we exceed the hard maximum duration that Wit.ai allows for a single speech request
	// 2. If we exceed a user definable duration since we last received valid voice data

	const bool bIsTooLongSinceVoiceDataReceived = (LastVoiceTime >= Configuration->Voice.KeepAliveTime);
	const bool bIsTooLongSinceActivated = (LastWakeTime >= Configuration->Voice.MaximumRecordingTime);
	const bool bShouldDeactivate = (bIsTooLongSinceVoiceDataReceived || bIsTooLongSinceActivated);
	
	if (bShouldDeactivate)
	{
		UE_LOG(LogWit, Display, TEXT("TickComponent: deactivating voice input - too long since activation (%d) - too long since voice input (%d)"), bIsTooLongSinceActivated, bIsTooLongSinceVoiceDataReceived);

		const bool bDidDeactivate = DoDeactivateVoiceInput();
		const bool bShouldCallStopEvent = bDidDeactivate && Events != nullptr;
		
		if (bShouldCallStopEvent)
		{
			if (bIsTooLongSinceActivated)
			{
				Events->OnStopVoiceInputDueToTimeout.Broadcast();
			}
			else if (bIsTooLongSinceVoiceDataReceived)
			{
				Events->OnStopVoiceInputDueToInactivity.Broadcast();
			}
		}
	}
}

/**
 * Set the configuration
 */
void UWitVoiceService::SetConfiguration(UWitAppConfigurationAsset* ConfigurationToUse, bool bUseWebSocketToUse)
{
	Super::SetConfiguration(ConfigurationToUse, bUseWebSocketToUse);

	UVoiceCaptureSubsystem* VoiceCaptureSubsystem = GEngine->GetEngineSubsystem<UVoiceCaptureSubsystem>();

	const bool bShouldEnableEmulation = VoiceCaptureSubsystem != nullptr && Configuration != nullptr && Configuration->Voice.EmulationCaptureMode != EVoiceCaptureEmulationMode::None;

	if (bShouldEnableEmulation)
	{
		VoiceCaptureSubsystem->EnableEmulation(Configuration->Voice.EmulationCaptureMode, Configuration->Voice.EmulationCaptureSoundWave, Configuration->Voice.TtsExperienceTag);
	}
}

/**
 * Starts receiving voice input from the microphone and begins streaming it to Wit.ai for interpretation
 *
 * @return true if the activation was successful
 */
bool UWitVoiceService::ActivateVoiceInput()
{
	// Configuration is required as we need the access token
	
	const bool bHasConfiguration = Configuration != nullptr && !Configuration->Application.ClientAccessToken.IsEmpty();
	
	if (!bHasConfiguration)
	{
		UE_LOG(LogWit, Warning, TEXT("ActivateVoiceInput: cannot active voice input because no configuration found. Please assign a configuration and access token"));
		return false;
	}
	
	// Prevent attempts to double activate this component
	
	if (bIsVoiceInputActive)
	{
		UE_LOG(LogWit, Warning, TEXT("ActivateVoiceInput: cannot activate voice input because it is already active on this component"));
		return false;
	}

	// Prevent attempts to activate more than one WitAPI component at a time by ensuring that all required subsystems are available and not in use
	
	UVoiceCaptureSubsystem* VoiceCaptureSubsystem = GEngine->GetEngineSubsystem<UVoiceCaptureSubsystem>();
	const UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();
	const bool bIsRequiredSubsystems = VoiceCaptureSubsystem != nullptr && RequestSubsystem != nullptr;
	
	if (!bIsRequiredSubsystems)
	{
		UE_LOG(LogWit, Warning, TEXT("ActivateVoiceInput: cannot activate voice input because required subsystems do not exist"));
		return false;
	}

	if (!VoiceCaptureSubsystem->IsCaptureAvailable())
	{
		// Attempt to startup the voice capture subsystem

		VoiceCaptureSubsystem->Startup();

		if (!VoiceCaptureSubsystem->IsCaptureAvailable())
		{
			UE_LOG(LogWit, Warning, TEXT("ActivateVoiceInput: cannot activate voice input because capture is not available"));
			return false;
		}
	}
	else if (VoiceCaptureSubsystem->IsCapturing())
	{
		UE_LOG(LogWit, Warning, TEXT("ActivateVoiceInput: cannot activate voice input because capture is already in progress"));
		return false;
	}

	if (RequestSubsystem->IsRequestInProgress())
	{
		UE_LOG(LogWit, Warning, TEXT("ActivateVoiceInput: cannot activate voice input because a request is already in progress"));
		return false;
	}
	
	bIsVoiceInputActive = VoiceCaptureSubsystem->Start();
	
	if (!bIsVoiceInputActive)
	{
		UE_LOG(LogWit, Warning, TEXT("ActivateVoiceInput: voice capture subsystem failed to start"));
		return false;
	}

	UE_LOG(LogWit, Display, TEXT("ActivateVoiceInput: activated voice input"));

	// Set the mic thresholds
	
	IConsoleVariable* SilenceDetectionThreshold = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.SilenceDetectionThreshold"));
	
	if (SilenceDetectionThreshold != nullptr)
	{
		SilenceDetectionThreshold->Set(Configuration->Voice.MicNoiseThreshold);
	}
	
	IConsoleVariable* NoiseGateThreshold = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.MicNoiseGateThreshold"));
	
	if (NoiseGateThreshold != nullptr)
	{
		NoiseGateThreshold->Set(Configuration->Voice.MicNoiseThreshold);
	}
	
	// We enable the tick in order to be able to handle auto-deactivation and reading data into the request

	SetComponentTickEnabled(true);
	
	LastVoiceTime = 0.0f;
	LastActivateTime = 0.0f;
	LastWakeTime = 0.0f;
	
	// Notify that we've started accepting voice input

	if (Events != nullptr)
	{
		Events->OnStartVoiceInput.Broadcast();
	}
	
	return true;
}

/**
 * Starts receiving voice input from the microphone and begins streaming it to Wit.ai for interpretation
 *
 * @return true if the activation was successful
 */
bool UWitVoiceService::ActivateVoiceInputWithRequestOptions(const FString& RequestOptions) 
{
	// TODO add RequestOptions into the url(or body), which likely will be only dynamic entities. check https://wit.ai/docs/http/20220622/#post__speech_link Arguments section. example post: https://wit.ai/docs/http/20220622/#post__entities__entity_keywords_link 

	UE_LOG(LogWit, Warning, TEXT("ActivateVoiceInputWithRequestOptions is not implemented yet, will use ActivateVoiceInput for now."));

	return ActivateVoiceInput();
}

/**
 * Starts receiving voice input from the microphone and begins streaming it to Wit.ai for interpretation
 *
 * @return true if the activation was successful
 */
bool UWitVoiceService::ActivateVoiceInputImmediately()
{
	const bool bIsActivated = ActivateVoiceInput();

	if (bIsActivated)
	{
		BeginStreamRequest();
	}

	return bIsActivated;
}

/**
 * Starts receiving voice input from the microphone and begins streaming it to Wit.ai for interpretation
 *
 * @return true if the activation was successful
 */
bool UWitVoiceService::ActivateVoiceInputImmediatelyWithRequestOptions(const FString& RequestOptions) 
{
	// TODO add RequestOptions into the url(or body), which likely will be only dynamic entities. check https://wit.ai/docs/http/20220622/#post__speech_link Arguments section. example post: https://wit.ai/docs/http/20220622/#post__entities__entity_keywords_link 

	UE_LOG(LogWit, Warning, TEXT("ActivateVoiceInputImmediatelyWithRequestOptions is not implemented yet, will use ActivateVoiceInputImmediately for now."));

	return ActivateVoiceInputImmediately();
}

/**
 * Start a streamed Wit request
 */
void UWitVoiceService::BeginStreamRequest()
{
	UE_LOG(LogWit, Display, TEXT("BeginStreamRequest: starting stream request"));

	if (bUseWebSocket)
	{
		UWitSocketSubsystem* SocketSubsystem = GEngine->GetEngineSubsystem<UWitSocketSubsystem>();

		const TSharedPtr<FJsonObject> RequestBody = MakeShared<FJsonObject>();
		RequestBody->SetStringField("content_type", "audio/raw;bits=16;rate=16k;encoding=signed-integer;endian=little");
		SocketSubsystem->SendJsonData(ERequestType::Converse, RequestBody.ToSharedRef());
	}
	else
	{
		const UVoiceCaptureSubsystem* VoiceCaptureSubsystem = GEngine->GetEngineSubsystem<UVoiceCaptureSubsystem>();
		UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();
#ifdef CPP_PLUGIN
#if PLATFORM_ANDROID
		folly::InlineExecutor InlineExecutor;
		folly::Executor::KeepAlive<> ExecutorToken;
		ExecutorToken = getKeepAliveToken(InlineExecutor);

		std::shared_ptr<IVoiceSdkApi> VoiceApi = std::make_shared<VoiceSdkApi>(
			nullptr, // TODO: Replace with implementation of IVoiceSdkCallbacListener
			ExecutorToken);

		std::shared_ptr<RequestParameterFactory> ParameterFactory = std::make_shared<RequestParameterFactory>();
		std::shared_ptr<IComposerRequestParameter> TtsRequestParameter = ParameterFactory->createComposerRequestParameter();
		std::shared_ptr<InputProviderFactory> ProviderFactory = std::make_shared<InputProviderFactory>();


		StreamInputProvider = ProviderFactory->createAudioStreamInputProvider();
		if (!StreamInputProvider)
		{
			UE_LOG(LogWit, Error, TEXT("ConvertTextToSpeechWithSettings: Streaming not yet supported via CPP_PLUGIN"));
			return;
		}
		StreamInputProvider->setAudioFormat(InputAudioFormat::RAW);
		StreamInputProvider->setBits(16);
		StreamInputProvider->setEncoding(AudioEncoding::SignedInteger);
		StreamInputProvider->setEndian(Endian::LittleEndian);
		StreamInputProvider->setSampleRate(VoiceCaptureSubsystem->SampleRate);
#endif
#else

		// Construct the request with the desired configuration. We use the /speech endpoint in Wit.ai. See the Wit.ai documentation for more
		// specifics of the parameters to this endpoint

		FWitRequestConfiguration RequestConfiguration{};

		FWitRequestBuilder::SetRequestConfigurationWithDefaults(RequestConfiguration, EWitRequestEndpoint::Speech, Configuration->Application.ClientAccessToken,
			Configuration->Application.Advanced.ApiVersion, Configuration->Application.Advanced.URL);
		FWitRequestBuilder::AddFormatContentType(RequestConfiguration, Format);
		FWitRequestBuilder::AddEncodingContentType(RequestConfiguration, Encoding);
		FWitRequestBuilder::AddSampleSizeContentType(RequestConfiguration, SampleSize);
		FWitRequestBuilder::AddRateContentType(RequestConfiguration, VoiceCaptureSubsystem->SampleRate);
		FWitRequestBuilder::AddEndianContentType(RequestConfiguration, EWitRequestEndian::Little);

		RequestConfiguration.bShouldUseCustomHttpTimeout = Configuration->Application.Advanced.bIsCustomHttpTimeout;
		RequestConfiguration.HttpTimeout = Configuration->Application.Advanced.HttpTimeout;

		RequestConfiguration.OnRequestError.AddUObject(this, &UWitVoiceService::OnWitRequestError);
		RequestConfiguration.OnRequestProgress.AddUObject(this, &UWitVoiceService::OnSpeechRequestProgress);
		RequestConfiguration.OnRequestComplete.AddUObject(this, &UWitVoiceService::OnSpeechRequestComplete);

		if (Events != nullptr)
		{
			Events->OnRequestCustomize.ExecuteIfBound(RequestConfiguration);
		}
		// Begin a streamed request to Wit.ai. For a streamed request we open an HTTP request to the server and continually write data as it
		// becomes available. This greatly reduces latency over waiting for the whole voice data and then sending it

		RequestSubsystem->BeginStreamRequest(RequestConfiguration);
#endif
	}

	bIsVoiceStreamingActive = true;

	// Notify that we've started sending voice data

	if (Events != nullptr)
	{
		Events->OnMinimumWakeThresholdHit.Broadcast();
	}

#if WITH_EDITORONLY_DATA
	
	if (Configuration->Voice.bIsWavFileRecordingEnabled)
	{
		RecordedVoiceInputBuffer.Reset();
	}

#endif
}

/**
 * Stops receiving voice input from the microphone and stops streaming it to Wit.ai
 *
 * @return true if the deactivation was successful
 */
bool UWitVoiceService::DeactivateVoiceInput()
{
	const bool bDidDeactivate = DoDeactivateVoiceInput();
	const bool bShouldCallStopEvent = bDidDeactivate && Events != nullptr;
	
	if (bShouldCallStopEvent)
	{
		Events->OnStopVoiceInputDueToDeactivation.Broadcast();
	}

	return bDidDeactivate;
}

bool UWitVoiceService::DeactivateAndAbortRequest() 
{
	// TODO also unbind all delegates
	
	return DeactivateVoiceInput();
}

/**
 * Internal function that does the main work of deactivating voice input
 *
 * @return true if the deactivation was successful
 */
bool UWitVoiceService::DoDeactivateVoiceInput()
{
	if (!bIsVoiceInputActive)
	{
		UE_LOG(LogWit, Warning, TEXT("DeactivateVoiceInput: cannot deactivate voice input because it is not active on this component"));
		return false;
	}
	
	UVoiceCaptureSubsystem* VoiceCaptureSubsystem = GEngine->GetEngineSubsystem<UVoiceCaptureSubsystem>();
	const bool bIsVoiceCapturing = VoiceCaptureSubsystem != nullptr && VoiceCaptureSubsystem->IsCapturing();
	
	if (bIsVoiceCapturing)
	{
		VoiceCaptureSubsystem->Stop();
	}
	else
	{
		UE_LOG(LogWit, Warning, TEXT("DeactivateVoiceInput: cannot deactivate voice capture because capture is not in progress"));
	}

	// End the streamed request. This will tell the HTTP client to send any remaining data and the close the request

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();
	const bool bIsRequestInProgress = RequestSubsystem != nullptr && RequestSubsystem->IsRequestInProgress();
	
	if (bIsRequestInProgress)
	{
#ifdef CPP_PLUGIN
#if PLATFORM_ANDROID
		if (!StreamInputProvider)
		{
			UE_LOG(LogWit, Error, TEXT("DoDeactivateVoiceInput: Not Initialized"));
			return false;
		}
		StreamInputProvider->writeEndOfStream();
#endif
#else
		RequestSubsystem->EndStreamRequest();
#endif
	}
	else
	{
		UE_LOG(LogWit, Warning, TEXT("DeactivateVoiceInput: cannot end stream request input because request is not in progress"));
	}

	UE_LOG(LogWit, Display, TEXT("DeactivateVoiceInput: deactivated voice input"));
	
	// We disable the tick as it is only really needed when voice input is activate to handle auto-deactivation
	
	SetComponentTickEnabled(false);

	bIsVoiceInputActive = false;
	bIsVoiceStreamingActive = false;
	
	// Notify that we've stopped accepting voice input

	if (Events != nullptr)
	{
		Events->OnStopVoiceInput.Broadcast();
	}
	
	// If desired write the recorded data to a wav file

#if WITH_EDITORONLY_DATA
	
	if (Configuration->Voice.bIsWavFileRecordingEnabled)
	{
		WriteRawPCMDataToWavFile(RecordedVoiceInputBuffer.GetData(), RecordedVoiceInputBuffer.Num(), VoiceCaptureSubsystem->NumChannels, VoiceCaptureSubsystem->SampleRate);
	}

#endif
	
	return true;
}

/**
 * Is voice input active on this component?
 *
 * @return true if active otherwise false
 */
bool UWitVoiceService::IsVoiceInputActive() const
{
	return bIsVoiceInputActive;
}

/**
 * Get the current voice input volume
 *
 * @return if voice capture is in progress returns the current volume otherwise 0
 */
float UWitVoiceService::GetVoiceInputVolume() const
{
	if (!bIsVoiceInputActive)
	{
		return 0.0f;
	}

	const UVoiceCaptureSubsystem* VoiceCaptureSubsystem = GEngine->GetEngineSubsystem<UVoiceCaptureSubsystem>();

	if (VoiceCaptureSubsystem == nullptr)
	{
		return 0.0f;
	}

	return VoiceCaptureSubsystem->GetCurrentAmplitude();
}

/**
 * Is voice streaming to Wit.ai active on this component?
 *
 * @return true if active otherwise false
 */
bool UWitVoiceService::IsVoiceStreamingActive() const
{
	return bIsVoiceStreamingActive;
}

/**
 * Is any Wit.ai request currently in progress?
 *
 * @return true if in progress otherwise false
 */
bool UWitVoiceService::IsRequestInProgress() const
{
	const UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();
	const bool bIsRequestInProgress = RequestSubsystem != nullptr && RequestSubsystem->IsRequestInProgress();

	return bIsRequestInProgress;
}

/**
 * Sends a text string to Wit for interpretation.
 *
 * @param Text [in] The string we want to interpret
 */
void UWitVoiceService::SendTranscription(const FString& Text)
{	
	const bool bHasConfiguration = Configuration != nullptr && !Configuration->Application.ClientAccessToken.IsEmpty();
	
	if (!bHasConfiguration)
	{
		UE_LOG(LogWit, Warning, TEXT("SendTranscription: cannot send transcription because no configuration found. Please assign a configuration and access token"));
		return;
	}

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	if (RequestSubsystem == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("SendTranscription: cannot send transcription because request subsystem does not exist"));
		return;
	}

	if (RequestSubsystem->IsRequestInProgress())
	{
		UE_LOG(LogWit, Warning, TEXT("SendTranscription: cannot send transcription because a request is already in progress"));
		return;
	}

	UE_LOG(LogWit, Display, TEXT("SendTranscription: sending transcription (%s)"), *Text);

	// Construct the request with the desired configuration. We use the /message endpoint in Wit.ai. See the Wit.ai documentation for more
	// specifics of the parameters to this endpoint

#ifdef CPP_PLUGIN
#if PLATFORM_ANDROID
	folly::InlineExecutor InlineExecutor;
	folly::Executor::KeepAlive<> ExecutorToken;
	ExecutorToken = getKeepAliveToken(InlineExecutor);

	const std::shared_ptr<IVoiceSdkApi> VoiceApi = std::make_shared<VoiceSdkApi>(
		nullptr, // TODO: Replace with implementation of IVoiceSdkCallbacListener
		ExecutorToken);

	std::shared_ptr<RequestParameterFactory> ParameterFactory = std::make_shared<RequestParameterFactory>();
	std::shared_ptr<ICommandRequestParameter> CommandRequestParameter = ParameterFactory->createCommandRequestParameter();

	std::shared_ptr<InputProviderFactory> ProviderFactory = std::make_shared<InputProviderFactory>();
	std::shared_ptr<ITextStaticInputProvider> StaticInputProvider = ProviderFactory->createTextStaticInputProvider();
	StaticInputProvider->setText(TCHAR_TO_UTF8(*Text));
	const FGuid Guid = FGuid::NewGuid();
	const FString RequestId = *Guid.ToString(EGuidFormats::Digits);
	VoiceApi->activate(
		std::string(TCHAR_TO_UTF8(*RequestId)), std::move(CommandRequestParameter), StaticInputProvider);


#endif
#else
	FWitRequestConfiguration RequestConfiguration{};

	FWitRequestBuilder::SetRequestConfigurationWithDefaults(RequestConfiguration, EWitRequestEndpoint::Message, Configuration->Application.ClientAccessToken,
		Configuration->Application.Advanced.ApiVersion, Configuration->Application.Advanced.URL);

	const FString EncodedText = FGenericPlatformHttp::UrlEncode(Text);
	FWitRequestBuilder::AddParameter(RequestConfiguration, EWitParameter::Text, EncodedText);

	RequestConfiguration.bShouldUseCustomHttpTimeout = Configuration->Application.Advanced.bIsCustomHttpTimeout;
	RequestConfiguration.HttpTimeout = Configuration->Application.Advanced.HttpTimeout;

	RequestConfiguration.OnRequestError.AddUObject(this, &UWitVoiceService::OnWitRequestError);
	RequestConfiguration.OnRequestComplete.AddUObject(this, &UWitVoiceService::OnMessageRequestComplete);

	if (Events != nullptr)
	{
		Events->OnRequestCustomize.ExecuteIfBound(RequestConfiguration);
	}
	
	RequestSubsystem->BeginStreamRequest(RequestConfiguration);
	RequestSubsystem->EndStreamRequest();
#endif
}

/**
 * Sends a text string to Wit for interpretation
 *
 * @param RequestOptions [in] the RequestOptions string, this could be used for Dynamic Entities
 * @param Text [in] the string we want to interpret
 */
void UWitVoiceService::SendTranscriptionWithRequestOptions(const FString& Text, const FString& RequestOptions)
{
	// TODO: add RequestOptions into the url(or body), which likely will be only dynamic entities. check https://wit.ai/docs/http/20220622/#post__speech_link Arguments section. example post: https://wit.ai/docs/http/20220622/#post__entities__entity_keywords_link 

	UE_LOG(LogWit, Warning, TEXT("SendTranscriptionWithRequestOptions is not implemented yet, will use SendTranscription for now."));
	SendTranscription(Text);
}

/**
 * Accept the given Partial Response and cancel the current request
 *
 * @param Response [in] the Partial Response to accept, this will be used as final response to call onResponse.
 */
void UWitVoiceService::AcceptPartialResponseAndCancelRequest(const FWitResponse& Response)
{
#ifdef CPP_PLUGIN
#if PLATFORM_ANDROID
	if (!StreamInputProvider)
	{
		UE_LOG(LogWit, Error, TEXT("DoDeactivateVoiceInput: Not Initialized"));
		return;
	}
	StreamInputProvider->writeEndOfStream();
#endif
#else
	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	if (RequestSubsystem == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("SendTranscription: cannot send transcription because request subsystem does not exist"));
		return;
	}

	RequestSubsystem->CancelRequest();
#endif

	DeactivateVoiceInput();

	FWitResponse FinalResponse = Response;
	FinalResponse.Is_Final = true;

	OnRequestComplete(FinalResponse);
}

/**
 * Called when a Wit speech request is in progress with any partial transcriptions
 *
 * @param PartialBinaryResponse [in] the partial response as binary
 * @param PartialJsonResponse [in] the partial response as Json
 */
void UWitVoiceService::OnSpeechRequestProgress(const TArray<uint8>& PartialBinaryResponse, const TSharedPtr<FJsonObject> PartialJsonResponse) const
{
	UE_LOG(LogWit, Verbose, TEXT("OnSpeechRequestProgress: %d"), PartialBinaryResponse.Num());
	// The text field of the final response chunk represents the most recent transcription that Wit.ai was able to discern. We pass this to the user
	// registered callback as it can be used to display intermediate partial transcriptions which make the application feel more responsive

	if (!PartialJsonResponse)
	{
		return;
	}
	if (FWitHelperUtilities::IsWitResponse(PartialJsonResponse))
	{
		OnPartialResponse(PartialBinaryResponse, PartialJsonResponse);
	}
	else
	{
		const FString PartialTranscription = PartialJsonResponse->GetStringField(TEXT("text"));

		if (Events != nullptr)
		{
			Events->OnPartialTranscription.Broadcast(PartialTranscription);
		}
	}
}

/**
 *  Called when received a Wit partial response
 *
 * @param PartialBinaryResponse [in] the partial binary response
 * @param PartialJsonResponse [in] the partial Json response
 */
void UWitVoiceService::OnPartialResponse(const TArray<uint8>& PartialBinaryResponse, const TSharedPtr<FJsonObject> PartialJsonResponse) const
{
	if (Events == nullptr)
	{
		return;		
	}
	
	Events->WitResponse.Reset();
	
	const bool bIsConversionError = !FWitHelperUtilities::ConvertJsonToWitResponse(PartialJsonResponse.ToSharedRef(), &Events->WitResponse);
	
	if (bIsConversionError)
	{
		OnWitRequestError(TEXT("Json To UStruct failed"), TEXT("Convering the Json partial response to a UStruct failed"));
		return;
	}

	Events->OnWitPartialResponse.Broadcast(true, Events->WitResponse);
}

/**
 * Called when a Wit message(Transcription) request is successfully completed to process the final response payload
 *
 * @param BinaryResponse [in] the final binary response
 * @param JsonResponse [in] the final Json response
 */
void UWitVoiceService::OnMessageRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	
	Events->WitResponse.Reset();
	Events->WitResponse.Is_Final = true;
	OnRequestComplete(BinaryResponse, JsonResponse, false);
}

/**
 * Called when a Wit speech request is successfully completed to process the final response payload
 *
 * @param BinaryResponse [in] the final binary response
 * @param JsonResponse [in] the final Json response
 */
void UWitVoiceService::OnSpeechRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	
	OnRequestComplete(BinaryResponse, JsonResponse, true);
}

/**
 * Called when a Wit voice request is successfully completed to process the final response payload
 *
 * @param BinaryResponse [in] the final binary response
 * @param JsonResponse [in] the final Json response
 * @param bIsResponseRestNeeded [in] whether to reset the Response Object.
 * 
 */
void UWitVoiceService::OnRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse, const bool bIsResponseRestNeeded)
{
	if (Events == nullptr)
	{
		return;		
	}
	if (bIsResponseRestNeeded)
	{
		Events->WitResponse.Reset();
	}
	
	
	const bool bIsConversionError = !FWitHelperUtilities::ConvertJsonToWitResponse(JsonResponse.ToSharedRef(), &Events->WitResponse);
	if (bIsConversionError)
	{
		OnWitRequestError(TEXT("Json To UStruct failed"), TEXT("Converting the Json response to a UStruct failed"));
		return;
	}
	
	OnRequestComplete(Events->WitResponse);
}

/**
 * Called when a Wit voice request is successfully completed to process the final response payload
 *
 * @param Response [in] the final binary response
 */
void UWitVoiceService::OnRequestComplete(const FWitResponse& Response) const
{
	if (Events == nullptr)
	{
		return;		
	}
	
	Events->WitResponse = Response;

	UE_LOG(LogWit, Display, TEXT("Full transcription received (%s)"), *Events->WitResponse.Text);
	UE_LOG(LogWit, Verbose, TEXT("UStruct - Text: %s"), *Events->WitResponse.Text);

	Events->OnFullTranscription.Broadcast(Events->WitResponse.Text);
	Events->OnWitResponse.Broadcast(true, Events->WitResponse);
}

/**
 * Called when the state of a WebSocket connection changes
 */
void UWitVoiceService::OnSocketStateChange()
{
	UWitSocketSubsystem* SocketSubsystem = GEngine->GetEngineSubsystem<UWitSocketSubsystem>();
	ESocketState SocketStatus = SocketSubsystem->GetSocketState();
	FString Status = *UEnum::GetValueAsString(SocketStatus);
	UE_LOG(LogWit, Verbose, TEXT("OnSocketStateChange %s"), *Status);

	if (SocketStatus == ESocketState::Disconnected)
	{
		SocketSubsystem->CreateSocket(Configuration->Application.ClientAccessToken);
	}
}

/**
 * Called when a Wit request errors
 *
 * @param ErrorMessage [in] the error message
 * @param HumanReadableErrorMessage [in] a longer human readable error message
 */
void UWitVoiceService::OnWitRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage) const
{
	UE_LOG(LogWit, Warning, TEXT("Wit request failed with error: %s - %s"), *ErrorMessage, *HumanReadableErrorMessage);

	if (Events != nullptr)
	{
		Events->WitResponse.Reset();
		Events->OnWitResponse.Broadcast(false, Events->WitResponse);
		Events->OnWitError.Broadcast(ErrorMessage, HumanReadableErrorMessage);
	}
}

#if WITH_EDITORONLY_DATA

/**
 * Write the captured voice input to a wav file. The output file will be written to the project folder's Saved/BouncedWavFiles folder as
 * Wit/RecordedVoiceInput.wav. This only works with 16-bit samples
 */
void UWitVoiceService::WriteRawPCMDataToWavFile(const uint8* RawPCMData, int32 RawPCMDataSize, int32 NumChannels, int32 SampleRate)
{
	RecordingData.Reset(new Audio::FAudioRecordingData());
	RecordingData->InputBuffer = Audio::TSampleBuffer<int16>(reinterpret_cast<const int16*>(RawPCMData), RawPCMDataSize / 2, NumChannels, SampleRate);

	const FString WavFileName = TEXT("RecordedVoiceInput");
	FString WavFilePath = TEXT("Wit");
	
	RecordingData->Writer.BeginWriteToWavFile(RecordingData->InputBuffer, WavFileName, WavFilePath, []()
	{
		RecordingData.Reset();
	});
}

#endif
