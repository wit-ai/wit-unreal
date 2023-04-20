/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Voice/Configuration/VoiceConfiguration.h"
#include "VoiceCaptureSubsystem.generated.h"

class USoundWave;

/**
 * UE4 Voice Capture wrapper for Wit.ai integration
 */
UCLASS()
class UVoiceCaptureSubsystem final : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:

	/**
	 * Initialize the subsystem. USubsystem override
	 *
	 * @return true if successfully initialized
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * De-initializes the subsystem. USubsystem override
	 *
	 * @return true if successfully initialized
	 */
	virtual void Deinitialize() override;

	/**
	 * Prepare the voice capture for use
	 */
	void Startup();

	/**
	 * Shutdown the voice capture after use. After calling this the voice capture cannot be used again
	 * until another call to Startup is made
	 */
	void Shutdown();
	
	/**
	 * Starts capturing voice data. Captured voice data is buffered in the internal buffer
	 * up until the maximum specified duration
	 *
	 * @return true if successfully started
	 */
	bool Start();

	/**
	 * Reads data from the voice capture module into the internal buffer for processing
	 * This should typically be called every frame
	 *
	 * @return true if any data was read
	 */
	bool Read();

	/**
	 * Returns the current amplitude of the voice capture
	 *
	 * @return the current amplitude
	 */
	float GetCurrentAmplitude() const;
	
	/**
	 * Stop capturing voice data. Should be paired with Start
	 */
	void Stop();

	/**
	 * Is the subsystem currently available to capture?
	 *
	 * @return true if currently available
	 */
	bool IsCaptureAvailable() const;
	
	/**
	 * Are we currently capturing voice data?
	 *
	 * @return true if currently capturing
	 */
	bool IsCapturing() const;

	/**
	 * Enable the use of the null capture
	 */
	UFUNCTION()
	void EnableEmulation(EVoiceCaptureEmulationMode EmulationModeToUse, USoundWave* SoundWaveToUse, const FName& Tag);

	/**
	 * Get read access to the latest voice data
	 *
	 * @return reference to the voice data
	 */
	const TArray<uint8>& GetVoiceBuffer() const;
	
	/** The sample rate of the captured voice data. UE4 by default captures at 8k. Changing this rate may cause it not to function correctly */
	const int32 SampleRate{16000};

	/** The number of channels in the captured voice data. UE4 by default captures 1 channel. Changing this value may cause it to not function correctly */
	const int32 NumChannels{1};

	/** The maximum duration (in seconds) of the voice data we can store in our internal buffer */
	const int32 MaxDuration{1};

private:

	/**
	  * Callback when the application enters the background. Used to shutdown the voice capture
	  */
	void OnApplicationWillEnterBackground();
	
	/**
	  * Callback when modules are loaded/unloaded. Used to track when the voice capture module gets loaded/unloaded
	  */
	void OnModulesChanged(FName ModuleName, EModuleChangeReason ChangeReason);

    /**
      * Creates the internal UE4 voice capture object and prepares it for use
      *
      * @return true if successfully created
      */
	bool CreateVoiceCapture();

	/**
	  * Create the emulation voice capture
	  */
	void CreateEmulationVoiceCapture();
	
	/** UE4's voice capture implementation */
	TSharedPtr<class IVoiceCapture> VoiceCapture{};

	/** The size in bytes of the internal buffer we use for storing voice data */
	int32 MaxBufferSize{0};

	/** Buffer that stores the captured voice data */
	TArray<uint8> VoiceBuffer{};

	/** Allow the use of emulation if unable to initialise mic input */
	EVoiceCaptureEmulationMode EmulationCaptureMode{EVoiceCaptureEmulationMode::None};

	/** Sound wave to use with the null capture */
	UPROPERTY()
	USoundWave* EmulationCaptureSoundWave{};
	
	/** Set Tag for TTS speaker which is used to create sound wave from TTS to use with the null capture */
	UPROPERTY()
	FName TtsExperienceTag{};
};
