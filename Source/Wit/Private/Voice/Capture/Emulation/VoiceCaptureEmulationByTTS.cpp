/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "VoiceCaptureEmulationByTTS.h"

#include "AudioDecompress.h"
#include "AudioDevice.h"
#include "Engine.h"
#include "Wit/Utilities/WitLog.h"
#include "Wit/Utilities/WitHelperUtilities.h"

/**
 * Constructor for the null voice capture
 */
FVoiceCaptureEmulationByTts::FVoiceCaptureEmulationByTts()
{
	// Deliberately empty
}

/**
 * Destructor for the null voice capture
 */
FVoiceCaptureEmulationByTts::~FVoiceCaptureEmulationByTts()
{
	// Deliberately empty
}

/**
 * Initialise the voice capture
 */
bool FVoiceCaptureEmulationByTts::Init(const FString& DeviceName, int32 SampleRate, int32 NumChannels)
{
	return true;
}

/**
 * Shutdown the voice capture
 */
void FVoiceCaptureEmulationByTts::Shutdown()
{
	Stop();
}

void FVoiceCaptureEmulationByTts::OnSynthesizeRawResponse(const TArray<uint8>& BinaryData)
{
	const USoundWave* SoundWave = FWitHelperUtilities::CreateSoundWaveFromRawData(BinaryData.GetData(), BinaryData.Num(), EWitRequestAudioFormat::Wav, false);

	if (SoundWave == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("FVoiceCaptureEmulationByTts: starting with no soundwave"));
		ProduceSoundDuration = OutputSoundDuration;
		return;
	}
	ProduceSoundDuration = SoundWave->Duration;
	
	DecompressedRawPCMDataSize = BinaryData.Num();
	DecompressedRawPCMData.SetNumZeroed(DecompressedRawPCMDataSize);
	FMemory::Memcpy(DecompressedRawPCMData.GetData(), BinaryData.GetData(), DecompressedRawPCMDataSize);
	
	bIsProducingSound = true;
}

/**
 * Start capturing voice input
 */
bool FVoiceCaptureEmulationByTts::Start()
{
	bIsCapturing = true;
	ProduceSoundTimer = 0.0f;
	
	const FWorldContext* World = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
	const ATtsExperience* TtsExperience = FWitHelperUtilities::FindTtsExperience(World->World(), TtsExperienceTag);
	if (TtsExperience != nullptr && TtsExperience->EventHandler != nullptr)
	{
		TtsExperience->EventHandler->OnSynthesizeRawResponseMulticast.AddRaw(this, &FVoiceCaptureEmulationByTts::OnSynthesizeRawResponse);
	}
	else
	{
		UE_LOG(LogWit, Warning, TEXT("Cannot find a TTS Speaker for VoiceCaptureEmulation, will have no voice input, please add a TTS Speaker first."));
	}

	return true;
	
}

/**
 * Stop capturing voice input
 */
void FVoiceCaptureEmulationByTts::Stop()
{
	bIsCapturing = false;
}

/**
 * Change capture device
 */
bool FVoiceCaptureEmulationByTts::ChangeDevice(const FString& DeviceName, int32 SampleRate, int32 NumChannels)
{
	 return true;
}

/**
 * Are we currently capturing?
 */
bool FVoiceCaptureEmulationByTts::IsCapturing()
{
	return bIsCapturing;
}

/**
 * Get the current capture state
 */
EVoiceCaptureState::Type FVoiceCaptureEmulationByTts::GetCaptureState(uint32& OutAvailableVoiceData) const
{
	OutAvailableVoiceData = UncompressedAudioBuffer.Num();
	
	if (!bIsCapturing)
	{
		OutAvailableVoiceData = 0;
		return EVoiceCaptureState::NotCapturing;
	}

	if (!bIsProducingSound)
	{
		OutAvailableVoiceData = 0;
		return EVoiceCaptureState::NoData;
	}

	return EVoiceCaptureState::Ok;
}

/**
 * Get the latest available voice data
 */
EVoiceCaptureState::Type FVoiceCaptureEmulationByTts::GetVoiceData(uint8* OutVoiceBuffer, uint32 InVoiceBufferSize, uint32& OutAvailableVoiceData)
{
	uint64 UnusedSampleCounter = 0;
	return GetVoiceData(OutVoiceBuffer, InVoiceBufferSize, OutAvailableVoiceData, UnusedSampleCounter);
}

/**
 * Get the latest available voice data
 */
EVoiceCaptureState::Type FVoiceCaptureEmulationByTts::GetVoiceData(uint8* OutVoiceBuffer, uint32 InVoiceBufferSize, uint32& OutAvailableVoiceData, uint64& OutSampleCounter)
{
	EVoiceCaptureState::Type CaptureState = GetCaptureState(OutAvailableVoiceData);

	OutAvailableVoiceData = 0;
	OutSampleCounter = 0;
	
	if (CaptureState == EVoiceCaptureState::Ok)
	{
		if (InVoiceBufferSize >= static_cast<uint32>(UncompressedAudioBuffer.Num()))
		{
			OutAvailableVoiceData = UncompressedAudioBuffer.Num();
			
			FMemory::Memcpy(OutVoiceBuffer, UncompressedAudioBuffer.GetData(), OutAvailableVoiceData);
			UncompressedAudioBuffer.Reset();
		}
		else
		{
			CaptureState = EVoiceCaptureState::BufferTooSmall;
		}
	}
	
	return CaptureState;
}

/**
 * Get the maximum possible buffer size
 */
int32 FVoiceCaptureEmulationByTts::GetBufferSize() const
{
	return 2048;
}

/**
 * Log out a dump of the current state
 */
void FVoiceCaptureEmulationByTts::DumpState() const
{
	// Deliberately empty
}

/**
 * @return the current loudness of the microphone
 */
float FVoiceCaptureEmulationByTts::GetCurrentAmplitude() const
{
	if (bIsCapturing && bIsProducingSound)
	{
		return 1.0f;
	}

	return 0.0f;
}

/**
 * Per frame tick function
 */
bool FVoiceCaptureEmulationByTts::Tick(float DeltaTime)
{
	const float LastProduceSoundTimer = ProduceSoundTimer;
	
	if (bIsCapturing && bIsProducingSound)
	{
		ProduceSoundTimer += DeltaTime;

		if (ProduceSoundTimer >= ProduceSoundDuration)
		{
			ProduceSoundTimer = ProduceSoundDuration;
		}
	}

	if (!bIsProducingSound)
	{
		UncompressedAudioBuffer.Reset();
		
		return true;
	}
	
	if (ProduceSoundDuration > 0)
	{
		const uint64 ElementCount = DecompressedRawPCMDataSize;
		// Use the sound wave samples to generate capture data
		
		const uint64 LastSampleOffset = LastProduceSoundTimer / ProduceSoundDuration * static_cast<float>(ElementCount) / sizeof(int16);
		const uint64 SampleOffset = ProduceSoundTimer / ProduceSoundDuration * static_cast<float>(ElementCount) / sizeof(int16);

		uint64 LastDataIndex = LastSampleOffset * sizeof(int16);

		if (LastDataIndex > ElementCount)
		{
			LastDataIndex = ElementCount;
		}
		
		uint64 DataIndex = SampleOffset * sizeof(int16);

		if (DataIndex > ElementCount)
		{
			DataIndex = ElementCount;
		}
		
		const uint64 DataSizeToCopy = DataIndex - LastDataIndex;
		const bool bIsAnyDataToCopy = DataSizeToCopy > 0;
		
		UncompressedAudioBuffer.Reset(DataSizeToCopy);

		if (bIsAnyDataToCopy)
		{
			UncompressedAudioBuffer.AddUninitialized(DataSizeToCopy);
			FMemory::Memcpy(UncompressedAudioBuffer.GetData(), &DecompressedRawPCMData[LastDataIndex], DataSizeToCopy);
		}
	}
	else
	{
		// Generate N seconds of sound and then silence

		for (auto i = 0; i < OutputSamplesPerFrame; ++i)
		{
			UncompressedAudioBuffer.Add(0xff);
		}
	}

	// This gets set after the data copy so that we don't chop off some of the final data
	
	if (ProduceSoundTimer >= ProduceSoundDuration)
	{
		bIsProducingSound = false;
	}
	
	return true;
}

/**
 * Set the TTS Experience tag to use
 */
void FVoiceCaptureEmulationByTts::SetTtsExperienceTag(const FName& TtsExperienceTagToUse)
{
	TtsExperienceTag = TtsExperienceTagToUse;
}

