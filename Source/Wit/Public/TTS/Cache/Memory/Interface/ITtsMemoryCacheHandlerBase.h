/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "TTS/Configuration/TtsConfiguration.h"

class USoundWave;

/**
 * Interface for implementing a memory cache. Can be overriden to implement custom functionality
 */
class ITtsMemoryCacheHandlerBase
{
public:

	/** 
	 * Destructor for overrides 
	 */
	virtual ~ITtsMemoryCacheHandlerBase() = default;
	
	/**
	 * Add a clip to the cache
	 *
	 * @param ClipId [in] the clip id
	 * @param SoundWave [in] the sound wave
	 * @param ClipSettings [in] the settings that were originally used to create the clip
	 *
	 * @return true if the clip was added
	 */
	virtual bool AddClip(const FString& ClipId, USoundWave* SoundWave, const FTtsConfiguration& ClipSettings) = 0;

	/**
	 * Remove a clip from the cache
	 *
	 * @param ClipId [in] the clip id
	 *
	 * @return true if the clip was removed
	 */
	virtual bool RemoveClip(const FString& ClipId) = 0;

	/**
	 * Remove all clips from the cache
	 */	
	virtual void RemoveAllClips() = 0;

	/**
	 * Get a clip given its id
	 *
	 * @param ClipId [in] the clip id
	 *
	 * @return the clip's sound wave
	 */	
	virtual USoundWave* GetClip(const FString& ClipId) const = 0;

	/**
	 * Get all the currently cached clips
	 * 
	 * @return all the clip sound waves
	 */	
	virtual TArray<USoundWave*> GetClips() const = 0;
	
};
