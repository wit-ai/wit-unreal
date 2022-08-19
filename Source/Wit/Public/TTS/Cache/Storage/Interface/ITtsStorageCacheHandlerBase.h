/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "TTS/Configuration/TtsConfiguration.h"

/**
 * Interface for implementing a storage cache. Can be overriden to implement custom functionality
 */
class ITtsStorageCacheHandlerBase
{
public:

	/** 
	 * Destructor for overrides 
	 */
	virtual ~ITtsStorageCacheHandlerBase() = default;

	/**
	 * Get the path to the given clip in the cache
	 *
	 * @param CacheLocation [in] the cache location where the clip will be 
	 * @param CachePath [out] the full path to the clip in the cache
	 *
	 * @return false if the path does not exist or cannot be created
	 */
	virtual bool GetCachePath(const ETtsStorageCacheLocation CacheLocation, FString& CachePath) const = 0;

	/**
	 * Should we cache a clip?
	 *
	 * @param CacheLocation [in] the cache location where the clip will be 
	 *
	 * @return false if the path does not exist or cannot be created
	 */
	virtual bool ShouldCache(const ETtsStorageCacheLocation CacheLocation) const = 0;

	/**
	 * Add a clip to the cache
	 *
	 * @param ClipId [in] the clip id
	 * @param ClipData [in] the binary data that represents the clip
	 * @param ClipSettings [in] the settings that were originally used to create the clip
	 *
	 * @return true is the clip was added
	 */
	virtual bool AddClip(const FString& ClipId, const TArray<uint8>& ClipData, const FTtsConfiguration& ClipSettings) = 0;

	/**
	 * Request a clip from the cache
	 *
	 * @param ClipId [in] the clip id
	 * @param CacheLocation [in] the cache location where the clip will be 
	 * @param ClipData [out] the binary data that represents the clip
	 *
	 * @return true if the clip is available in the cache
	 */
	virtual bool RequestClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation, TArray<uint8>& ClipData) const = 0;

	/**
	 * Remove a clip from the cache
	 *
	 * @param ClipId [in] the clip id
	 * @param CacheLocation [in] the cache location where the clip will be
	 *
	 * @return true if the clip was removed
	 */
	virtual bool RemoveClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation) = 0;

	/**
	 * Remove all clips from the cache
	 * @param CacheLocation [in] the cache location where the clip will be
	 */	
	virtual void RemoveAllClips(const ETtsStorageCacheLocation CacheLocation) = 0;
	
};
