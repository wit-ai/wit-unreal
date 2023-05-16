/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Interfaces/VoiceCapture.h"
#include "Misc/EngineVersionComparison.h"
#include "Sound/SoundWave.h"

/**
 * Null implementation of voice capture. This just returns 1 second of known input to hit the wake volume and then silence after
 */
#if UE_VERSION_OLDER_THAN(5,0,0)
class FVoiceCaptureEmulation final : public IVoiceCapture, public FTickerObjectBase
#else
class FVoiceCaptureEmulation final : public IVoiceCapture, public FTSTickerObjectBase
#endif
{
public:

	FVoiceCaptureEmulation();
	virtual ~FVoiceCaptureEmulation() override;

	/**
	 * IVoiceCapture overrides
	 */
	virtual bool Init(const FString& DeviceName, int32 SampleRate, int32 NumChannels) override;
	virtual void Shutdown() override;
	virtual bool Start() override;
	virtual void Stop() override;
	virtual bool ChangeDevice(const FString& DeviceName, int32 SampleRate, int32 NumChannels) override;
	virtual bool IsCapturing() override;
	virtual EVoiceCaptureState::Type GetCaptureState(uint32& OutAvailableVoiceData) const override;
	virtual EVoiceCaptureState::Type GetVoiceData(uint8* OutVoiceBuffer, uint32 InVoiceBufferSize, uint32& OutAvailableVoiceData) override;
	virtual EVoiceCaptureState::Type GetVoiceData(uint8* OutVoiceBuffer, uint32 InVoiceBufferSize, uint32& OutAvailableVoiceData, uint64& OutSampleCounter) override;
	virtual int32 GetBufferSize() const override;
	virtual void DumpState() const override;
	virtual float GetCurrentAmplitude() const override;

	/**
	 * FTickerObjectBase overrides
	 */
	virtual bool Tick(float DeltaTime) override;

	/** Set the sound wave to use */
	void SetSoundWave(USoundWave* SoundWaveToUse);

private:

	/** The number of samples we will output per frame*/
	static constexpr int OutputSamplesPerFrame{4};
	
	/** The duration we will output sound for */
	static constexpr float OutputSoundDuration{1.0f};

	/** Are we currently capturing? */
	bool bIsCapturing{false};

	/** Are we currently outputting sound? */
	bool bIsProducingSound{false};

	/** How long have we been producing sound for */
	float ProduceSoundTimer{0.0f};

	/** How long we should produce sound for */
	float ProduceSoundDuration{OutputSoundDuration};

	/** Sound wave to propagate to the voice capture */
	USoundWave* SoundWave{};

	/** Uncompressed audio buffer */
	TArray<uint8> UncompressedAudioBuffer{};
  
    /** Whether the SoundWave has PreviewSampleData. SoundWave will have no PreviewSampleData(RawData) in packaged builds. For example running on Oculus */
    bool bHasPreviewSampleData{true};
  
    /** When bHasPreviewSampleData is false, the SoundWave is compressed, the RawData is null. In this case we will decompress it. DecompressedRawPCMData is for holding the decompressed data */
    TArray<uint8> DecompressedRawPCMData{};

	/** Holds the size of DecompressedRawPCMData. */
	uint32 DecompressedRawPCMDataSize{0};
};
