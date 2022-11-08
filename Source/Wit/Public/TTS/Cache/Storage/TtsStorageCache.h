/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "TTS/Cache/Storage/TtsStorageCacheHandler.h"
#include "TtsStorageCache.generated.h"

/**
 * Implements a simple storage cache backed on to disk
 */
UCLASS(ClassGroup=(Meta), meta=(BlueprintSpawnableComponent))
class WIT_API UTtsStorageCache final : public UTtsStorageCacheHandler
{
	GENERATED_BODY()

public:
	
	UTtsStorageCache();

	/**
	 * Get the path to the given clip in the cache
	 *
	 * @param CacheLocation [in] the cache location where the clip will be 
	 * @param CachePath [out] the full path to the clip in the cache
	 *
	 * @return false if the path does not exist or cannot be created
	 */
	virtual bool GetCachePath(const ETtsStorageCacheLocation CacheLocation, FString& CachePath) const override;

	/**
	 * Should we cache a clip?
	 *
	 * @param CacheLocation [in] the cache location where the clip will be 
	 *
	 * @return false if the path does not exist or cannot be created
	 */
	virtual bool ShouldCache(const ETtsStorageCacheLocation CacheLocation) const override;

	/**
	 * Add a clip to the cache
	 *
	 * @param ClipId [in] the clip id
	 * @param ClipData [in] the binary data that represents the clip
	 * @param ClipSettings [in] the settings that were originally used to create the clip
	 *
	 * @return true if the clip was added
	 */
	virtual bool AddClip(const FString& ClipId, const TArray<uint8>& ClipData, const FTtsConfiguration& ClipSettings) override;
	
	/**
	 * Request a clip from the cache
	 *
	 * @param ClipId [in] the clip id
	 * @param CacheLocation [in] the cache location where the clip will be 
	 * @param ClipData [out] the binary data that represents the clip
	 *
	 * @return true if the clip is available in the cache
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS")
	virtual bool RequestClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation, TArray<uint8>& ClipData) const override;

	/**
	 * Remove a clip from the cache
	 *
	 * @param ClipId [in] the clip id
	 * @param CacheLocation [in] the cache location where the clip will be
	 *
	 * @return true if the clip was removed
	 */
	UFUNCTION(BlueprintCallable, Category = "TTS")
	virtual bool RemoveClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation) override;

	/**
	 * Remove all clips from the cache
	 *
	 * @param CacheLocation [in] the cache location where the clip will be
	 */	
	UFUNCTION(BlueprintCallable, Category = "TTS")
	virtual void RemoveAllClips(const ETtsStorageCacheLocation CacheLocation) override;
	
	/**
	 * Cache directory relative to the base cache path
	 */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FString CacheDirectory{TEXT("Wit/Cache")};

	/**
	 * The default location to use for cached clips. Can be overriden on a per clip basis
	 */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	ETtsStorageCacheLocation DefaultCacheLocation{ETtsStorageCacheLocation::None};

private:

	/* Get the final location to cache a clip taking into account overrides */
	ETtsStorageCacheLocation GetFinalCacheLocation(const ETtsStorageCacheLocation CacheLocation) const;

};
