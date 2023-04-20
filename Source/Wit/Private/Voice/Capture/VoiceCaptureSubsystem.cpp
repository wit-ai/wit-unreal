/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Voice/Capture/VoiceCaptureSubsystem.h"
#include "VoiceModule.h"
#include "Emulation/VoiceCaptureEmulation.h"
#include "Emulation/VoiceCaptureEmulationByTTS.h"
#include "Wit/Utilities/WitConversionUtilities.h"
#include "Wit/Utilities/WitLog.h"
#include "Misc/EngineVersionComparison.h"
#if PLATFORM_ANDROID
#include "AndroidPermissionFunctionLibrary.h"
#include "AndroidPermissionCallbackProxy.h"
#include "Misc/CoreDelegates.h"
#endif

/**
 * Initialize the subsystem. USubsystem override
 */
void UVoiceCaptureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
#if PLATFORM_ANDROID
	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &UVoiceCaptureSubsystem::OnApplicationWillEnterBackground);
#endif
	FModuleManager::Get().OnModulesChanged().AddUObject(this,&UVoiceCaptureSubsystem::OnModulesChanged);
}

/**
 * De-initializes the subsystem. USubsystem override
 */
void UVoiceCaptureSubsystem::Deinitialize()
{
#if PLATFORM_ANDROID
	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.RemoveAll(this);
#endif
	FModuleManager::Get().OnModulesChanged().RemoveAll(this);
}

/**
 * Startup the voice capture for use. On Oculus this function also implements requesting any necessary
 * permissions in order to be able to access the device's microphone
 */
void UVoiceCaptureSubsystem::Startup()
{
	UE_LOG(LogWit, Display, TEXT("VoiceCapture - Init: init start - available (%d)"), FVoiceModule::IsAvailable());

#if PLATFORM_ANDROID
	UAndroidPermissionFunctionLibrary::Initialize();
	
	FString RecordPermission = "android.permission.RECORD_AUDIO";

	if (UAndroidPermissionFunctionLibrary::CheckPermission(RecordPermission))
	{
		UE_LOG(LogWit, Display, TEXT("VoiceCapture - Init: record permission already granted"));

		const bool bIsPlatformVoiceCaptureCreated = CreateVoiceCapture();

		if (!bIsPlatformVoiceCaptureCreated)
		{
			UE_LOG(LogWit, Display, TEXT("VoiceCapture - Init: could not create platform voice capture - using emulation instead"));

			if (EmulationCaptureMode != EVoiceCaptureEmulationMode::None)
			{
				CreateEmulationVoiceCapture();
			}
		}

		return;
	}

	TArray<FString> PermissionsToAcquire{ RecordPermission };
	UAndroidPermissionCallbackProxy* CallbackProxy = UAndroidPermissionFunctionLibrary::AcquirePermissions(PermissionsToAcquire);

	if (CallbackProxy)
	{
		
#if UE_VERSION_OLDER_THAN(5,0,0)
		CallbackProxy->OnPermissionsGrantedDelegate.BindLambda([this](const TArray<FString>& Permissions, const TArray<bool>& GrantResults)
#else
		CallbackProxy->OnPermissionsGrantedDelegate.AddLambda([this](const TArray<FString>& Permissions, const TArray<bool>& GrantResults)
#endif
		{
			bool bIsPermissionGranted = (GrantResults.Num() > 0 && GrantResults[0]);

			if (bIsPermissionGranted)
			{
				UE_LOG(LogWit, Display, TEXT("VoiceCapture - Init: record permission newly granted"));
			}
			else
			{
				UE_LOG(LogWit, Warning, TEXT("VoiceCapture - Init: record permission was denied"));
			}
		});
	}
#else

	const bool bIsPlatformVoiceCaptureCreated = CreateVoiceCapture();

	if (!bIsPlatformVoiceCaptureCreated)
	{
		UE_LOG(LogWit, Display, TEXT("VoiceCapture - Init: could not create platform voice capture - using null instead"));

		if (EmulationCaptureMode != EVoiceCaptureEmulationMode::None)
		{
			CreateEmulationVoiceCapture();
		}
	}

#endif

	UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - Init: finished"));
}

/**
 * Creates the underlying UE4 voice capture module that we use to retrieve microphone data. It also creates
 * the buffer that we use internally to store data that the voice capture module provides
 */
bool UVoiceCaptureSubsystem::CreateVoiceCapture()
{
	UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - CreateVoiceCapture: trying to create with sample rate (%d) and num channels (%d)"), SampleRate, NumChannels);

	// Force failure if we want to use emulation
	
	if (EmulationCaptureMode == EVoiceCaptureEmulationMode::AlwaysUseSoundWave || EmulationCaptureMode == EVoiceCaptureEmulationMode::AlwaysUseTTS)
	{
		return false;
	}
	
	if (!FVoiceModule::IsAvailable())
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - CreateVoiceCapture: voice module is not available - trying to load"));

		// Attempt to load the voice module in case it's not already loaded for some reason. If it still fails then we can't continue. We
		// don't use LoadModuleChecked because we don't want to assert if the load fails

		FModuleManager::LoadModulePtr<FVoiceModule>("Voice");
		
		if (!FVoiceModule::IsAvailable())
		{
			UE_LOG(LogWit, Warning, TEXT("VoiceCapture - CreateVoiceCapture: voice module is still not available"));
			return false;
		}
	}

	UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - CreateVoiceCapture: voice module is available"));
	
	if (!FVoiceModule::Get().IsVoiceEnabled())
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - CreateVoiceCapture: voice module does not have voice enabled"));
		return false;
	}

	UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - CreateVoiceCapture: voice module has voice enabled"));
	
	if (!FVoiceModule::Get().DoesPlatformSupportVoiceCapture())
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - CreateVoiceCapture: voice module is unsupported on this platform"));
		return false;
	}
	
	UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - CreateVoiceCapture: voice module is supported on this platform - default sample rate is %d"), UVOIPStatics::GetVoiceSampleRate());
	
	VoiceCapture = FVoiceModule::Get().CreateVoiceCapture(TEXT(""), SampleRate, NumChannels);
	if (!IsCaptureAvailable())
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - CreateVoiceCapture: failed to create Voice Capture"));
		return false;
	}

	VoiceCapture->DumpState();

	MaxBufferSize = VoiceCapture->GetBufferSize();
	VoiceBuffer.Reserve(MaxBufferSize);

	UE_LOG(LogWit, Verbose, TEXT("CreateVoiceCapture: voice capture is ready with max buffer size (%d)"), MaxBufferSize);

	return true;
}

/**
 * Create the emulation voice capture
 */
void UVoiceCaptureSubsystem::CreateEmulationVoiceCapture()
{
	TSharedPtr<IVoiceCapture> EmulationVoiceCapture ;
	if (EmulationCaptureMode == EVoiceCaptureEmulationMode::AlwaysUseSoundWave)
	{
		const TSharedPtr<FVoiceCaptureEmulation> VoiceCaptureEmulation = MakeShared<FVoiceCaptureEmulation>();
		VoiceCaptureEmulation->SetSoundWave(EmulationCaptureSoundWave);
		EmulationVoiceCapture = VoiceCaptureEmulation;
	}
	else
	{
		const TSharedPtr<FVoiceCaptureEmulationByTts> VoiceCaptureEmulationByTts = MakeShared<FVoiceCaptureEmulationByTts>();
		VoiceCaptureEmulationByTts->SetTtsExperienceTag(TtsExperienceTag);
		EmulationVoiceCapture = VoiceCaptureEmulationByTts;
	}
	MaxBufferSize = EmulationVoiceCapture->GetBufferSize();
	VoiceBuffer.Reserve(MaxBufferSize);

	VoiceCapture = EmulationVoiceCapture;
}

/**
 * Resets the voice capture after finishing. After calling this it is no longer valid to call Start/Read/Stop
 */
void UVoiceCaptureSubsystem::Shutdown()
{
	if (!IsCaptureAvailable())
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - Reset: voice capture ptr is not valid"));
		return;
	}

	if (IsCapturing())
	{
		VoiceCapture->Stop();
	}

	VoiceCapture->Shutdown();
	VoiceCapture.Reset();

	UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - Reset: shutdown voice capture"));
}

/**
 * Indicates that we want to start receiving data from the voice capture module
 */
bool UVoiceCaptureSubsystem::Start()
{
	if (!IsCaptureAvailable())
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - Start: voice capture ptr is not valid. Make sure it is setup correctly"));
		return false;
	}

	if (IsCapturing())
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - Start: attempting to start capture when it has already been started"));
		return false;
	}

	UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - Start: starting capture"));

	VoiceBuffer.Reset();
	return VoiceCapture->Start();
}

/**
 * Reads data from the voice capture module into our internal buffer. The amount of data read by each call to Read
 * is variable and depends on the platform and how often the function is called
 *
 * @return true if successfully read
 */
bool UVoiceCaptureSubsystem::Read()
{
	if (!IsCaptureAvailable())
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - Read: voice capture ptr is not valid. Make sure it is setup correctly"));
		return false;
	}

	if (!IsCapturing())
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - Read: attempting to read voice data before capture has been started"));
		return false;
	}

	// Determine whether and how much new voice data is available to be used

	uint32 NumAvailableBytes = 0;
	uint32 NumOutputBytes = 0;

	const EVoiceCaptureState::Type State = VoiceCapture->GetCaptureState(NumAvailableBytes);

	const bool bIsCaptureSuccessful = (State == EVoiceCaptureState::Ok);
	if (!bIsCaptureSuccessful)
	{
		if (State != EVoiceCaptureState::NoData)
		{
			UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - Read: capture state is not ok (%s)"), EVoiceCaptureState::ToString(State));
		}

		return false;
	}

	const bool bIsNewDataAvailable = (NumAvailableBytes > 0);
	if (!bIsNewDataAvailable)
	{
		return false;
	}

#if PLATFORM_ANDROID

	// On Android this indicates useless noise data that accumulated during standby. We still need to request it as it will 
	// reset the circular buffer and discard the data but won't actually write anything to our buffer. The size 4096 is
	// arbitrary but big enough to trigger the special case code in the Android voice module

	bool bIsNoiseDataToBeDiscarded = (NumAvailableBytes > 2048);
	if (bIsNoiseDataToBeDiscarded)
	{
		UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - Read: discarding (%u) noise bytes"), NumAvailableBytes);

		VoiceBuffer.Reset();
		VoiceBuffer.AddUninitialized(MaxBufferSize);
		
		VoiceCapture->GetVoiceData(VoiceBuffer.GetData(), VoiceBuffer.Num(), NumOutputBytes);

		return false;
	}

#endif

	// Clamp the amount of bytes we request to our max buffer size. This prevents us using too much memory

	const bool bIsBufferOverrun = (NumAvailableBytes > static_cast<uint32>(MaxBufferSize));
	if (bIsBufferOverrun)
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - Read: recorded bytes exceeds available size - clamping"));
		NumAvailableBytes = MaxBufferSize;
	}

	// Set our buffer to the required size before reading in the data

	VoiceBuffer.Reset();
	VoiceBuffer.AddUninitialized(NumAvailableBytes);

	VoiceCapture->GetVoiceData(VoiceBuffer.GetData(), VoiceBuffer.Num(), NumOutputBytes);

	UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - Read: read (%u) bytes, output (%u) bytes"), NumAvailableBytes, NumOutputBytes);

	return true;
}

/**
 * Stop receiving voice capture data
 */
void UVoiceCaptureSubsystem::Stop()
{
	if (!IsCaptureAvailable())
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - Stop: voice capture ptr is not valid. Make sure it is setup correctly"));
		return;
	}

	if (!IsCapturing())
	{
		UE_LOG(LogWit, Warning, TEXT("VoiceCapture - Stop: attempting to stop before capture has been started"));
		return;
	}

	VoiceCapture->Stop();

	UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - Stop: stopping capture"));
}

/**
 * Returns the current amplitude of the voice capture
 *
 * @return the current amplitude
 */
float UVoiceCaptureSubsystem::GetCurrentAmplitude() const
{
	if (!IsCapturing())
	{
		return 0.0f;
	}

	const float Amplitude = VoiceCapture->GetCurrentAmplitude();

	// GetCurrentAmplitude returns a value of -1.0 if there is no amplitude information available. This happens on certain platforms
	
	const bool bIsAmplitudeAvailable = Amplitude >= 0.0f;
	if (bIsAmplitudeAvailable)
	{
		return Amplitude;
	}

	// On some platforms the amplitude is not available. This seems to apply to the Android voice implementation. As a fallback we
	// scan the current voice buffer and find the highest amplitude currently in the buffer

	const int32 NumSamples = VoiceBuffer.Num() / sizeof(int16);
	return FWitConversionUtilities::CalculateMaximumAmplitude16Bit(VoiceBuffer.GetData(), NumSamples);
}

/**
 * Is the subsystem currently available to capture?
 *
 * @return true if currently available
 */
bool UVoiceCaptureSubsystem::IsCaptureAvailable() const
{
	return VoiceCapture.IsValid();
}

/**
 * Are we currently capturing voice data?
 *
 * @return true if currently capturing
 */
bool UVoiceCaptureSubsystem::IsCapturing() const
{
	return IsCaptureAvailable() && VoiceCapture->IsCapturing();
}

/**
 * Get read access to the latest voice data
 *
 * @return reference to the voice data
 */
const TArray<uint8>& UVoiceCaptureSubsystem::GetVoiceBuffer() const
{
	return VoiceBuffer;
}

/**
 * Enable use of the null capture
 */
void UVoiceCaptureSubsystem::EnableEmulation(EVoiceCaptureEmulationMode EmulationModeToUse, USoundWave* SoundWaveToUse, const FName& TtsExperienceTagToUse)
{
	EmulationCaptureMode = EmulationModeToUse;
	EmulationCaptureSoundWave = SoundWaveToUse;
	TtsExperienceTag = TtsExperienceTagToUse;
}

/**
  * Callback when the application enters the background. Used to shutdown the voice capture
  */
void UVoiceCaptureSubsystem::OnApplicationWillEnterBackground()
{
	UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - OnApplicationWillEnterBackground"));

	Shutdown();
}

/**
  * Callback when modules are loaded/unloaded. Used to track when the voice capture module gets loaded/unloaded
  */
void UVoiceCaptureSubsystem::OnModulesChanged(FName ModuleName, EModuleChangeReason ChangeReason)
{
	const bool bIsVoiceModule = ModuleName.IsEqual("Voice");
	if (!bIsVoiceModule)
	{
		return;
	}

	if (ChangeReason == EModuleChangeReason::ModuleLoaded)
	{
		UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - OnModulesChanged: module has been loaded"));
	}
	else if (ChangeReason == EModuleChangeReason::ModuleUnloaded)
	{
		UE_LOG(LogWit, Verbose, TEXT("VoiceCapture - OnModulesChanged: module has been unloaded"));
	}
}
