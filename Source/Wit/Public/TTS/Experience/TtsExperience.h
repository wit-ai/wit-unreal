/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tts/Service/TtsService.h"
#include "Tts/Events/TtsEvents.h"
#include "Wit/Request/WitRequestTypes.h"
#include "Wit/Request/WitResponse.h"
#include "TtsExperience.generated.h"

/**
 * The base class of TtsExperience
 */
UCLASS(ClassGroup=(Meta), Abstract, NotBlueprintable)
class WIT_API ATtsExperience : public AActor, public ITtsServiceBase
{
	GENERATED_BODY()
	
public:

	/**
	 * Sets default values for this actor's properties
	 */
	ATtsExperience();

	/**
	 * ITtsService overrides
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	virtual bool IsRequestInProgress() const override;

	UFUNCTION(BlueprintCallable, Category="TTS")
	virtual void ConvertTextToSpeech(const FString& TextToConvert, bool bQueueAudio = true) override;

	UFUNCTION(BlueprintCallable, Category="TTS")
	virtual void ConvertTextToSpeechWithSettings(const FTtsConfiguration& ClipSettings, bool bQueueAudio = true) override;

	UFUNCTION(BlueprintCallable, Category="TTS")
	virtual void FetchAvailableVoices() override;
	
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
	 * The Wit TTS Audio Type that will be used by Wit.ai
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS")
	EWitRequestAudioFormat AudioType{EWitRequestAudioFormat::Wav};
	
	/**
	* Should the response be streamed by Wit.ai
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS")
	bool bUseStreaming{false};

	/**
	* How many seconds of data to buffer before playing audio.
	* Larger values introduce latency, but helps prevent audio stuttering
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS", meta = (EditCondition = "bUseStreaming", EditConditionHides))
	float InitialStreamBufferSize{ 0.02f };

	/**
	* Should WebSocket be used to commuicate with Wit.ai
	*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS", meta = (EditCondition = "false", EditConditionHides))
	bool bUseWebSocket{false};

	/**
	 * The events used by the voice service
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TTS")
	UTtsEvents* EventHandler{};
	
	/**
	 * Memory cache to store converted voice clips as USoundWave
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TTS Cache")
	UTtsMemoryCacheHandler* MemoryCacheHandler{};

	/**
	 * Storage cache to store converted voice clips as raw data
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TTS Cache")
	UTtsStorageCacheHandler* StorageCacheHandler{};

	/**
	 * The underlying TTS service that implements the TTS functionality
	 */
	UPROPERTY()
	UTtsService* TtsService{};
	
protected:

	virtual void BeginPlay() override;

private:

	void InitializeService();
	
};
