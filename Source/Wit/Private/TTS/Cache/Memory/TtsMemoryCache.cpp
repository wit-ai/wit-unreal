/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TTS/Cache/Memory/TtsMemoryCache.h"
#include "Misc/EngineVersionComparison.h"
#include "Wit/Utilities/WitLog.h"
#include "Sound/SoundWave.h"

/**
 * Default constructor
 */
UTtsMemoryCache::UTtsMemoryCache()
{
	PrimaryComponentTick.bCanEverTick = false;
}

/**
 * Add a clip to the cache
 *
 * @param ClipId [in] the clip id
 * @param SoundWave [in] the sound wave
 * @param ClipSettings [in] the settings that were originally used to create the clip
 *
 * @return true if the clip was added
 */
bool UTtsMemoryCache::AddClip(const FString& ClipId, USoundWave* SoundWave, const FTtsConfiguration& ClipSettings)
{
	int32 Index;

	const bool bIsKnownClip = ClipIds.Find(ClipId, Index);
	if (bIsKnownClip)
	{
		RemoveClipAt(Index);
	}

	UE_LOG(LogWit, Verbose, TEXT("UTTSMemoryCache::AddClip: adding clip with text (%s) and id (%s)"), *ClipSettings.Text, *ClipId);
	
	ClipIds.Emplace(ClipId);	
	Clips.Emplace(SoundWave);
	ClipSettingsArray.Emplace(ClipSettings);
	
	while (IsFull())
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSMemoryCache::AddClip: cache is full - removing oldest clip"));
		
		RemoveClipAt(0);
	}

	if (!bIsKnownClip)
	{
		OnClipAdded.Broadcast(ClipId);
	}
	
	return !bIsKnownClip;
}

/**
 * Remove a clip from the cache
 *
 * @param ClipId [in] the clip id
 *
 * @return true if the clip was removed
 */	
bool UTtsMemoryCache::RemoveClip(const FString& ClipId)
{
	int32 Index;
	
	const bool bIsKnownClip = ClipIds.Find(ClipId, Index);
	if (!bIsKnownClip)
	{
		return false;	
	}

	RemoveClipAt(Index);

	return true;
}

/**
 * Remove all clips from the cache
 */	
void UTtsMemoryCache::RemoveAllClips()
{
	for (const auto& ClipId: ClipIds)
	{
		OnClipRemoved.Broadcast(ClipId);
	}
	
	ClipIds.Empty();
	Clips.Empty();
	ClipSettingsArray.Empty();
}

/**
 * Gets number of clips in cache
 *
 * @return number of clips in cache
 */	
int32 UTtsMemoryCache::GetUsedCacheClipCapacity() const
{
	return Clips.Num();
}

/**
 * Get a clip given its id
 *
 * @param ClipId [in] the clip id
 *
 * @return the clip's sound wave
 */	
USoundWave* UTtsMemoryCache::GetClip(const FString& ClipId) const
{
	int32 Index;

	UE_LOG(LogWit, Verbose, TEXT("UTTSMemoryCache::GetClip: requesting clip with id (%s)"), *ClipId);
	
	const bool bIsClipAdded = ClipIds.Find(ClipId, Index);
	if (!bIsClipAdded)
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSMemoryCache::GetClip: clip does not exist in cache"));
		return nullptr;
	}

	return Clips[Index];
}

/**
 * Get all the currently cached clips
 * 
 * @return all the clip sound waves
 */
TArray<USoundWave*> UTtsMemoryCache::GetClips() const
{
	return Clips;
}

/**
 * Removes a clip a the given array index
 */
void UTtsMemoryCache::RemoveClipAt(const int32 Index)
{
	UE_LOG(LogWit, Verbose, TEXT("UTTSMemoryCache::RemoveClipAt: removing clip at index (%d) with text (%s) and id (%s)"), Index, *ClipSettingsArray[Index].Text, *ClipIds[Index]);

	OnClipRemoved.Broadcast(*ClipIds[Index]);
	
	ClipIds.RemoveAt(Index);
	Clips.RemoveAt(Index);
	ClipSettingsArray.RemoveAt(Index);
}

/**
 * Has the cache reached it's capacity?
 */
bool UTtsMemoryCache::IsFull() const
{
	// We always want to allow one clip
	
	const bool bIsLastClip = Clips.Num() <= 1;
	if (bIsLastClip)
	{
		return false;
	}
	
	const bool bIsOverClipCapacity = bIsClipCapacityEnabled && Clips.Num() > ClipCapacity;
	if (bIsOverClipCapacity)
	{
		return true;
	}

	const bool bIsOverMemoryCapacity = bIsMemoryCapacityEnabled && GetUsedCacheSizeInKilobytes() > MemoryCapacityInKilobytes;
	if (bIsOverMemoryCapacity)
	{
		return true;
	}

	return false;
}

/**
 * Gets the amount of memory currently being used by the cache
 */
int32 UTtsMemoryCache::GetUsedCacheSizeInKilobytes() const
{
	int32 TotalSizeInBytes = 0;
	
	for (auto& Clip : Clips)
	{
#if UE_VERSION_OLDER_THAN(5,0,0)
        TotalSizeInBytes += Clip->ResourceSize;
#else
		TotalSizeInBytes += Clip->GetResourceSize();		
#endif
	}

	int32 TotalSizeInKilobytes = TotalSizeInBytes / 1024;
	const int32 RemainingBytes = TotalSizeInBytes % 1024;
	
	const bool bShouldRoundUp = RemainingBytes != 0;
	if (bShouldRoundUp)
	{
		++TotalSizeInKilobytes;
	}

	return TotalSizeInKilobytes;
}
