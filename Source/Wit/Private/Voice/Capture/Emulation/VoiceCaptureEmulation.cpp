/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "VoiceCaptureEmulation.h"

#include "AudioDecompress.h"
#include "AudioDevice.h"
#include "Engine.h"
#include "Interfaces/IAudioFormat.h"
#include "Wit/Utilities/WitLog.h"
#include "Misc/EngineVersionComparison.h"

/**
 * Constructor for the null voice capture
 */
FVoiceCaptureEmulation::FVoiceCaptureEmulation()
{
	// Deliberately empty
}

/**
 * Destructor for the null voice capture
 */
FVoiceCaptureEmulation::~FVoiceCaptureEmulation()
{
	// Deliberately empty
}

/**
 * Initialise the voice capture
 */
bool FVoiceCaptureEmulation::Init(const FString& DeviceName, int32 SampleRate, int32 NumChannels)
{
	return true;
}

/**
 * Shutdown the voice capture
 */
void FVoiceCaptureEmulation::Shutdown()
{
	Stop();
}

/**
 * Start capturing voice input
 */
bool FVoiceCaptureEmulation::Start()
{
	bIsCapturing = true;
	bIsProducingSound = true;
	ProduceSoundTimer = 0.0f;

	if (SoundWave != nullptr)
	{
		int64 SoundWaveElementSize = 0;
		
#if UE_VERSION_OLDER_THAN(5,1,0)
		SoundWaveElementSize = SoundWave->RawData.GetElementCount();
#elif WITH_EDITORONLY_DATA
		SoundWaveElementSize = SoundWave->RawData.GetPayloadSize();
#endif
		
		UE_LOG(LogWit, Verbose, TEXT("FVoiceCaptureEmulation: starting with soundwave with duration (%f), element count (%llu), RawPCMDataSize (%lu), bDecompressedFromOgg (%s)"),
			SoundWave->GetDuration(), SoundWaveElementSize, SoundWave->RawPCMDataSize, SoundWave->bDecompressedFromOgg? TEXT("yes"):TEXT("no"));

        if (SoundWaveElementSize == 0)
        {
			bHasPreviewSampleData = false;
			
			FAudioDevice* AudioDevice = GEngine ? GEngine->GetMainAudioDeviceRaw() : nullptr;

			if (AudioDevice == nullptr)
			{
				return false;
			}

			if (SoundWave == nullptr)
			{
				return false;
			}

			if (SoundWave->GetName() == TEXT("None"))
			{
				return false;
			}
#if UE_VERSION_OLDER_THAN(5, 3, 0)
			SoundWave->InitAudioResource(AudioDevice->GetRuntimeFormat(SoundWave));
			ICompressedAudioInfo* CompressedAudioInfo = AudioDevice->CreateCompressedAudioInfo(SoundWave);
#else
			SoundWave->InitAudioResource(SoundWave->GetRuntimeFormat());
			ICompressedAudioInfo* CompressedAudioInfo = IAudioInfoFactoryRegistry::Get().Create(SoundWave->GetRuntimeFormat());
#endif
			FSoundQualityInfo SoundQualityInfo = { 0 };

#if UE_VERSION_OLDER_THAN(5,0,0)
			if (CompressedAudioInfo->ReadCompressedInfo(SoundWave->ResourceData, SoundWave->ResourceSize, &SoundQualityInfo))
#else
        	if (CompressedAudioInfo->ReadCompressedInfo(SoundWave->GetResourceData(), SoundWave->GetResourceSize(), &SoundQualityInfo))
#endif
			{
				DecompressedRawPCMDataSize = SoundQualityInfo.SampleDataSize;
				DecompressedRawPCMData.SetNumZeroed(DecompressedRawPCMDataSize);
				CompressedAudioInfo->ExpandFile(DecompressedRawPCMData.GetData(), &SoundQualityInfo);
			}
        	
			delete CompressedAudioInfo;
        }
		ProduceSoundDuration = SoundWave->Duration;
	}
	else
	{
		UE_LOG(LogWit, Verbose, TEXT("FVoiceCaptureEmulation: starting with no soundwave"));
		
		ProduceSoundDuration = OutputSoundDuration;
	}
	
	return true;
}

/**
 * Stop capturing voice input
 */
void FVoiceCaptureEmulation::Stop()
{
	bIsCapturing = false;
}

/**
 * Change capture device
 */
bool FVoiceCaptureEmulation::ChangeDevice(const FString& DeviceName, int32 SampleRate, int32 NumChannels)
{
	 return true;
}

/**
 * Are we currently capturing?
 */
bool FVoiceCaptureEmulation::IsCapturing()
{
	return bIsCapturing;
}

/**
 * Get the current capture state
 */
EVoiceCaptureState::Type FVoiceCaptureEmulation::GetCaptureState(uint32& OutAvailableVoiceData) const
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
EVoiceCaptureState::Type FVoiceCaptureEmulation::GetVoiceData(uint8* OutVoiceBuffer, uint32 InVoiceBufferSize, uint32& OutAvailableVoiceData)
{
	uint64 UnusedSampleCounter = 0;
	return GetVoiceData(OutVoiceBuffer, InVoiceBufferSize, OutAvailableVoiceData, UnusedSampleCounter);
}

/**
 * Get the latest available voice data
 */
EVoiceCaptureState::Type FVoiceCaptureEmulation::GetVoiceData(uint8* OutVoiceBuffer, uint32 InVoiceBufferSize, uint32& OutAvailableVoiceData, uint64& OutSampleCounter)
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
int32 FVoiceCaptureEmulation::GetBufferSize() const
{
	return 2048;
}

/**
 * Log out a dump of the current state
 */
void FVoiceCaptureEmulation::DumpState() const
{
	// Deliberately empty
}

/**
 * @return the current loudness of the microphone
 */
float FVoiceCaptureEmulation::GetCurrentAmplitude() const
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
bool FVoiceCaptureEmulation::Tick(float DeltaTime)
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
	
	if (SoundWave != nullptr)
	{
#if UE_VERSION_OLDER_THAN(5,1,0)
		uint64 ElementCount = SoundWave->RawData.GetElementCount();
#elif UE_VERSION_OLDER_THAN(5,4,0) && WITH_EDITORONLY_DATA
		uint64 ElementCount = SoundWave->RawData.GetPayloadSize();
#else
		uint64 ElementCount = SoundWave->RawPCMDataSize;
#endif
		
		if (!bHasPreviewSampleData)
		{
			ElementCount = DecompressedRawPCMDataSize;
		}
		// Use the sound wave samples to generate capture data
		
		const uint64 LastSampleOffset = LastProduceSoundTimer / SoundWave->GetDuration() * static_cast<float>(ElementCount) / sizeof(int16);
		const uint64 SampleOffset = ProduceSoundTimer / SoundWave->GetDuration() * static_cast<float>(ElementCount) / sizeof(int16);

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
			if (bHasPreviewSampleData)// on editor
			{
#if UE_VERSION_OLDER_THAN(5,1,0)
				const uint8* SoundData = static_cast<const uint8*>(SoundWave->RawData.LockReadOnly());
				FMemory::Memcpy(UncompressedAudioBuffer.GetData(), &SoundData[LastDataIndex], DataSizeToCopy);
				SoundWave->RawData.Unlock();
#elif WITH_EDITORONLY_DATA
				const uint8* SoundData = static_cast<const uint8*>(SoundWave->RawData.GetPayload().Get().GetData());
				FMemory::Memcpy(UncompressedAudioBuffer.GetData(), &SoundData[LastDataIndex], DataSizeToCopy);
#endif
			}
			else
			{
				FMemory::Memcpy(UncompressedAudioBuffer.GetData(), &DecompressedRawPCMData[LastDataIndex], DataSizeToCopy);
			}
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
 * Set the sound wave to use
 */
void FVoiceCaptureEmulation::SetSoundWave(USoundWave* SoundWaveToUse)
{
	SoundWave = SoundWaveToUse;
}
