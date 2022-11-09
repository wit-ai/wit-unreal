/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "TTS/Configuration/TtsConfiguration.h"
#include "TTS/Service/TtsService.h"
#include "Wit/Request/WitResponse.h"
#include "WitTtsService.generated.h"

class FJsonObject;

/**
 * Component that encapsulates the Wit Text to Speech API. Provides functionality for speech synthesis from text input
 * using Wit.ai. To use it simply attach the UWitTtsService component in the hierarchy of any Actor
 */
UCLASS( ClassGroup=(Meta), meta=(BlueprintSpawnableComponent) )
class WIT_API UWitTtsService final : public UTtsService
{
	GENERATED_BODY()

public:
	
	/**
	 * Default constructor
	 */
	UWitTtsService();
	
	/**
	 * ITtsService overrides
	 */
	virtual bool IsRequestInProgress() const override;
	virtual void ConvertTextToSpeech(const FString& TextToConvert) override;
	virtual void ConvertTextToSpeechWithSettings(const FTtsConfiguration& ClipSettings) override;
	virtual void FetchAvailableVoices() override;

#if WITH_EDITORONLY_DATA

	/**
	 * If set to true this will record the voice input and write it to a named wav file for debugging. The output file will be written to
	 * the project folder's Saved/BouncedWavFiles folder as Wit/RecordedVoiceInput.wav
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS|Debug")
	bool bIsWavFileOutputEnabled{false};

#endif

protected:
	
    /** Called when the component is destroyed to do any cleanup */
	virtual void BeginDestroy() override;

private:

	/** Maximum length of text that can be passed to Wit.ai */
	static constexpr int MaximumTextLengthInRequest{280};

#if WITH_EDITORONLY_DATA
	
	/** Write the captured voice input to a wav file */
	static void WriteRawPCMDataToWavFile(const uint8* RawPCMData, const int32 RawPCMDataSize, const int32 NumChannels, const int32 SampleRate);

#endif
	
	/** Called when a storage cache request is fully completed to process the loaded data */
	void OnStorageCacheRequestComplete(const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings) const;

	/** Called when a Wit synthesize request is fully completed to process the response payload */
	void OnSynthesizeRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse) const;

	/** Called when a synthesize request errors */
	void OnSynthesizeRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage) const;

	/** Called when a Wit voices request is fully completed to process the response payload */
	void OnVoicesRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse);

	/** Called when a voices request errors */
	void OnVoicesRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage) const;

	/** Creates a sound wave from binary data and adds it to the memory cache */
	USoundWave* CreateSoundWaveAndAddToMemoryCache(const FString& ClipId, const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings) const;

	/** Last requested generation settings */
	FTtsConfiguration LastRequestedClipSettings{};

};
