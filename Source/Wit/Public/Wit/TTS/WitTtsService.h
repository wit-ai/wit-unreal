/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "TTS/Cache/Memory/TtsMemoryCacheHandler.h"
#include "TTS/Cache/Storage/TtsStorageCacheHandler.h"
#include "TTS/Configuration/TtsConfiguration.h"
#include "TTS/Configuration/TtsVoicePresetAsset.h"
#include "Wit/Configuration/WitAppConfigurationAsset.h"
#include "Wit/Request/WitResponse.h"
#include "WitTtsService.generated.h"

class FJsonObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSynthesizeResponseDelegate, const bool, bIsSuccessful, USoundWave*, SoundWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSynthesizeErrorDelegate, const FString&, ErrorMessage, const FString&, HumanReadableMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoicesResponseDelegate, const bool, bIsSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoicesErrorDelegate, const FString&, ErrorMessage, const FString&, HumanReadableMessage);

/**
 * Component that encapsulates the Wit Text to Speech API. Provides functionality for speech synthesis from text input
 * using Wit.ai. To use it simply attach the UWitTtsService component in the hierarchy of any Actor
 */
UCLASS( ClassGroup=(Meta), meta=(BlueprintSpawnableComponent) )
class WIT_API UWitTtsService final : public UActorComponent
{
	GENERATED_BODY()

public:
	
	/**
	 * Default constructor
	 */
	UWitTtsService();
		
	/**
	 * Is a Wit.ai request currently in progress?
	 *
	 * @return true if in progress otherwise false
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	bool IsRequestInProgress() const;
	
	/**
	 * Sends a text string to Wit for conversion to speech with default settings
	 *
	 * @param TextToConvert [in] the string we want to convert to speech
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void ConvertTextToSpeech(const FString& TextToConvert);

	/**
	 * Sends a text string to Wit for conversion to speech with custom settings
	 *
	 * @param ClipSettings [in] the string we want to convert to speech
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void ConvertTextToSpeechWithSettings(const FTtsConfiguration& ClipSettings);

	/**
	 * Fetch a list of available voices from Wit
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void FetchAvailableVoices();
	
	/**
	 * Unload a single clip from the memory cache
	 *
	 * @param ClipId [in] id of the clip to unload
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void UnloadClip(const FString& ClipId);
	
	/**
	 * Unload all clips from the memory cache
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void UnloadAllClips();
	
	/**
	 * Remove a single clip from the storage cache
	 *
	 * @param ClipId [in] id of the clip to delete
	 * @param CacheLocation [in] location of the clip
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void DeleteClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation);
	
	/**
	 * Remove all clips from the storage cache
	 *
	 * @param CacheLocation [in] location of the clip
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void DeleteAllClips(const ETtsStorageCacheLocation CacheLocation);
	
	/**
	 * The Wit configuration that will be used by Wit.ai
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS")
	UWitAppConfigurationAsset* Configuration{};
  
	/**
	 * The Wit TTS Voice Preset that will be used by Wit.ai
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS")
	UTtsVoicePresetAsset* VoicePreset{};

	/**
	 * The Wit TTS Voice Preset that will be used by Wit.ai
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS|Voices")
	FWitTtsVoicesResponse AvailableVoices{};

	/**
	  * Memory cache to store converted voice clips as USoundWave
	  */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TTS Cache")
	UTtsMemoryCacheHandler* MemoryCache{};

	/**
	 * Storage cache to store converted voice clips as raw data
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TTS Cache")
	UTtsStorageCacheHandler* StorageCache{};

#if WITH_EDITORONLY_DATA

	/**
	 * If set to true this will record the voice input and write it to a named wav file for debugging. The output file will be written to
	 * the project folder's Saved/BouncedWavFiles folder as Wit/RecordedVoiceInput.wav
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS|Debug")
	bool bIsWavFileOutputEnabled{false};

#endif
	
	/**
	 * Callback to call when a synthesize request has been fully processed. The callback receives a USoundWave containing the received wav
	 */
	UPROPERTY(BlueprintAssignable)
	FOnSynthesizeResponseDelegate OnSynthesizeResponse{};

	/**
	 * Callback to call when a synthesize error occurs
	 */
	UPROPERTY(BlueprintAssignable)
	FOnSynthesizeErrorDelegate OnSynthesizeError{};

	/**
	 * Callback to call when a voices request has been fully processed. The callback receives a list of available voices
	 */
	UPROPERTY(BlueprintAssignable)
	FOnVoicesResponseDelegate OnVoicesResponse{};

	/**
	 * Callback to call when a voices error occurs
	 */
	UPROPERTY(BlueprintAssignable)
	FOnVoicesErrorDelegate OnVoicesError{};

protected:
	
    /** Called when the component is destroyed to do any cleanup */
	virtual void BeginDestroy() override;

private:

	/** Maximum length of text that can be passed to Wit.ai */
	static constexpr int MaximumTextLengthInRequest{140};

#if WITH_EDITORONLY_DATA
	
	/** Write the captured voice input to a wav file */
	static void WriteRawPCMDataToWavFile(const uint8* RawPCMData, int32 RawPCMDataSize, int32 NumChannels, int32 SampleRate);

#endif
	
	/** Called when a storage cache request is fully completed to process the loaded data */
	void OnStorageCacheRequestComplete(const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings);

	/** Called when a Wit synthesize request is fully completed to process the response payload */
	void OnSynthesizeRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse);

	/** Called when a synthesize request errors */
	void OnSynthesizeRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage);

	/** Called when a Wit voices request is fully completed to process the response payload */
	void OnVoicesRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse);

	/** Called when a voices request errors */
	void OnVoicesRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage);

	/** Creates a sound wave from binary data and adds it to the memory cache */
	USoundWave* CreateSoundWaveAndAddToMemoryCache(const FString& ClipId, const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings);

	/** Last requested generation settings */
	FTtsConfiguration LastRequestedClipSettings{};
};
