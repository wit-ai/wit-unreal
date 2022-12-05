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
#include "Wit/TTS/WitTtsSpeaker.h"

/**
 * Null implementation of voice capture. This use TTS Experience's response sound wave as input. (This VoiceCaptureEmulation will pick a TTS Experience by the tag )
 */
class FVoiceCaptureEmulationByTts final: public IVoiceCapture

#if UE_VERSION_OLDER_THAN(5,0,0)
, public FTickerObjectBase
#else
, public FTSTickerObjectBase
#endif
{
public:

	FVoiceCaptureEmulationByTts();
	virtual ~FVoiceCaptureEmulationByTts() override;

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

	/** Set the TTS Speaker's tag to generate sound wave to use */
	void SetTtsExperienceTag(const FName& TtsExperienceTag);
	
protected:
	
	void OnSynthesizeRawResponse(const TArray<uint8>& BinaryData);
	
	bool LoadDataFromSoundWave();

private:

	/** The number of samples we will output per frame*/
	static constexpr int OutputSamplesPerFrame{4};
	
	/** The duration we will output sound for */
	static constexpr float OutputSoundDuration{1.0f};

	/** Are we currently capturing? */
	bool bIsCapturing{false};

	/** Are we currently outputting sound? */
	bool bIsProducingSound{false};

	
	bool bIsRawSoundWaveReady{false};
	bool bIsSoundWaveReady{false};

	/** How long have we been producing sound for */
	float ProduceSoundTimer{0.0f};

	/** How long we should produce sound for */
	float ProduceSoundDuration{OutputSoundDuration};

	/** TTS Speaker to generate sound wave to propagate to the voice capture */
	FName TtsExperienceTag{};

	/** Uncompressed audio buffer */
	TArray<uint8> UncompressedAudioBuffer{};
  
    /** When bHasPreviewSampleData is false, the SoundWave is compressed, the RawData is null. In this case we will decompress it. DecompressedRawPCMData is for holding the decompressed data */
    TArray<uint8> DecompressedRawPCMData{};

	/** Holds the size of DecompressedRawPCMData. */
	uint32 DecompressedRawPCMDataSize{0};
};
