/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "TtsMemoryCacheHandler.h"
#include "TTS/Configuration/TtsConfiguration.h"
#include "Wit/Request/WitRequestTypes.h"
#include "TtsMemoryCache.generated.h"

class USoundWave;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClipAddedDelegate, const FString, ClipId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClipRemovedDelegate, const FString, ClipId);

/**
 * Implements a simple memory cache that has capacity controls and LRU ejection policy
 */
UCLASS(ClassGroup=(Meta), meta=(BlueprintSpawnableComponent))
class WIT_API UTtsMemoryCache final : public UTtsMemoryCacheHandler
{
	GENERATED_BODY()

public:

	UTtsMemoryCache();

	/**
	 * Add a clip to the cache
	 *
	 * @param ClipId [in] the clip id
	 * @param SoundWave [in] the sound wave
	 * @param ClipSettings [in] the settings that were originally used to create the clip
	 *
	 * @return true if the clip was added
	 */
	virtual bool AddClip(const FString& ClipId, USoundWave* SoundWave, const FTtsConfiguration& ClipSettings) override;

	/**
	 * Remove a clip from the cache
	 *
	 * @param ClipId [in] the clip id
	 *
	 * @return true if the clip was removed
	 */	
	virtual bool RemoveClip(const FString& ClipId) override;

	/**
	 * Remove all clips from the cache
	 */	
	virtual void RemoveAllClips() override;

	/**
	 * Get a clip given its id
	 *
	 * @param ClipId [in] the clip id
	 *
	 * @return the clip's sound wave
	 */	
	virtual USoundWave* GetClip(const FString& ClipId) const override;

	/**
	 * Get all the currently cached clips
	 * 
	 * @return all the clip sound waves
	 */	
	virtual TArray<USoundWave*> GetClips() const override;

	/**
	 * Is the clip capacity in use or not?
	 */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capacity")
	bool bIsClipCapacityEnabled{true};

	/**
	 * Maximum number of clips that can be stored at any one time in the cache
	 */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capacity")
	int32 ClipCapacity{100};

	/**
	 * Is the memory capacity in use or not?
	 */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capacity")
	bool bIsMemoryCapacityEnabled{false};

	/**
	 * Maximum amount of clip memory that can be used at any one time by the cache
	 */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capacity")
	int32 MemoryCapacityInKilobytes{1024};

	/**
	 * Callback to call when a clip is added to the memory cache
	 */
	UPROPERTY(BlueprintAssignable)
	FOnClipAddedDelegate OnClipAdded{};
	
	/**
	 * Callback to call when a clip is removed from the memory cache
	 */
	UPROPERTY(BlueprintAssignable)
	FOnClipRemovedDelegate OnClipRemoved{};

	/**
	 * Gets number of clips in cache
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	int32 GetUsedCacheClipCapacity() const;
	
	/**
	 * Gets the amount of memory currently being used by the cache
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	int32 GetUsedCacheSizeInKilobytes() const;
	
private:

	/**
	 * Has the cache reached it's capacity?
	 */
	bool IsFull() const;

	/**
	 * Removes a clip a the given array index
	 */
	void RemoveClipAt(const int32 Index);

	/**
	 * All the clip ids currently stored
	 */
	UPROPERTY(VisibleAnywhere, Transient, Category = "Contents")
	TArray<FString> ClipIds{};

	/**
	 * All the wav clips currently stored
	 */
	UPROPERTY(VisibleAnywhere, Transient, Category = "Contents")
	TArray<USoundWave*> Clips{};

	/**
	 * All the generation settings currently stored
	 */
	UPROPERTY(VisibleAnywhere, Transient, Category = "Contents")
	TArray<FTtsConfiguration> ClipSettingsArray{};
};
