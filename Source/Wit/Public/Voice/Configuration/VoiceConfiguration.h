/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "VoiceConfiguration.generated.h"

class USoundWave;

/**
 * Emulation modes
 */
UENUM()
enum class EVoiceCaptureEmulationMode : uint8
{
	None,
	AlwaysUseSoundWave, //TODO disable this for UE5
	AlwaysUseTTS
	// OnMicFail
};

/**
 * Voice configuration for /speech endpoint of Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FVoiceConfiguration
{
	GENERATED_BODY()

	/**
	 * The mic noise threshold
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation", meta=(ClampMin = 0, ClampMax = 1))
	float MicNoiseThreshold{0.01f};

	/**
	 * Until we reach this minimum voice volume the data will not start streaming to Wit.ai
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation", meta=(ClampMin = 0, ClampMax = 1))
	float WakeMinimumVolume{0.01f};

	/**
	 * Until we reach this amount of time the data will not start streaming to Wit.ai
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation", meta=(ClampMin = 0, ClampMax = 10))
	float WakeMinimumTime{0.5f};

	/**
	 * The minimum voice volume for keeping the voice input active
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keep Alive", meta=(ClampMin = 0, ClampMax = 1))
	float KeepAliveMinimumVolume{0.02f};

	/**
	 * If we don't receive any voice input for this amount of time when we automatically deactivate the voice input 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keep Alive", meta=(ClampMin = 0))
	float KeepAliveTime{2.0f};
	
	/**
	 * If the voice input goes on longer than this then we automatically deactivate.
	 * 20 seconds is the hard limit for the /speech endpoint
	 * 300 seconds is the hard limit for the /dictation endpoint
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keep Alive", meta=(ClampMin = 0, ClampMax = 300))
	float MaximumRecordingTime{20.0f};

	/**
	 * If set to true this will record the voice input and write it to a named wav file for debugging. The output file will be written to
	 * the project folder's Saved/BouncedWavFiles folder as Wit/RecordedVoiceInput.wav
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bIsWavFileRecordingEnabled{false};

	/**
	 * If set to true this will allow the use of the null voice capture as a fallback if the mic capture cannot be found
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	EVoiceCaptureEmulationMode EmulationCaptureMode{};

	/**
	 * If set then specifies the sound wave to use for null voice capture
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	USoundWave* EmulationCaptureSoundWave{};
	/**
	 * If set then the TtsExperience's response will be used for null voice capture
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	FName TtsExperienceTag{};
};
