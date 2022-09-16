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
	UFUNCTION(BlueprintCallable, Category="Experience")
	virtual bool IsRequestInProgress() const override;

	UFUNCTION(BlueprintCallable, Category="Experience")
	virtual void ConvertTextToSpeech(const FString& TextToConvert) override;

	UFUNCTION(BlueprintCallable, Category="Experience")
	virtual void ConvertTextToSpeechWithSettings(const FTtsConfiguration& ClipSettings) override;

	UFUNCTION(BlueprintCallable, Category="Experience")
	virtual void FetchAvailableVoices() override;
	
	/**
	 * Unload a single clip from the memory cache
	 *
	 * @param ClipId [in] id of the clip to unload
	 */
	UFUNCTION(BlueprintCallable, Category="Experience")
	void UnloadClip(const FString& ClipId);
	
	/**
	 * Unload all clips from the memory cache
	 */
	UFUNCTION(BlueprintCallable, Category="Experience")
	void UnloadAllClips();
	
	/**
	 * Remove a single clip from the storage cache
	 *
	 * @param ClipId [in] id of the clip to delete
	 * @param CacheLocation [in] location of the clip
	 */
	UFUNCTION(BlueprintCallable, Category="Experience")
	void DeleteClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation);
	
	/**
	 * Remove all clips from the storage cache
	 *
	 * @param CacheLocation [in] location of the clip
	 */
	UFUNCTION(BlueprintCallable, Category="Experience")
	void DeleteAllClips(const ETtsStorageCacheLocation CacheLocation);

	/**
	 * The Wit configuration that will be used by Wit.ai
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience|Configuration")
	UWitAppConfigurationAsset* Configuration{};
  
	/**
	 * The Wit TTS Voice Preset that will be used by Wit.ai
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience|Configuration")
	UTtsVoicePresetAsset* VoicePreset{};
	
	/**
	 * The events used by the voice service
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Experience|Handlers")
	UTtsEvents* EventHandler{};
	
	/**
	 * Memory cache to store converted voice clips as USoundWave
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Handlers")
	UTtsMemoryCacheHandler* MemoryCacheHandler{};

	/**
	 * Storage cache to store converted voice clips as raw data
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Handlers")
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
