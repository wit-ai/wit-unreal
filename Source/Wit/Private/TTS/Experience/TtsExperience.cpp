/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TTS/Experience/TtsExperience.h"
#include "CoreMinimal.h"
#include "TTS/Cache/Memory/TtsMemoryCache.h"
#include "TTS/Cache/Storage/TtsStorageCache.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Sets default values for this actor's properties
 */
ATtsExperience::ATtsExperience()
	: Super()
{
	// Create handlers

	EventHandler = CreateDefaultSubobject<UTtsEvents>(TEXT("EventHandler"));
	MemoryCacheHandler = CreateDefaultSubobject<UTtsMemoryCache>(TEXT("MemoryCache"));
	StorageCacheHandler = CreateDefaultSubobject<UTtsStorageCache>(TEXT("StorageCache"));
}

/**
 * Called when play begins
 */
void ATtsExperience::BeginPlay()
{
	InitializeService();
	
	Super::BeginPlay();
}

/**
 * Initialize the service
 */
void ATtsExperience::InitializeService()
{
	if (TtsService != nullptr)
	{
		TtsService->SetHandlers(EventHandler, MemoryCacheHandler, StorageCacheHandler);
		TtsService->SetConfiguration(Configuration, VoicePreset, AudioType, bUseStreaming, InitialStreamBufferSize, bUseWebSocket);
	}
}

/**
 * Is a Wit.ai request currently in progress?
 *
 * @return true if in progress otherwise false
 */
bool ATtsExperience::IsRequestInProgress() const
{
	if (TtsService != nullptr)
	{
		return TtsService->IsRequestInProgress();
	}

	return false;
}

/**
 * Sends a text string to Wit for conversion to speech with default settings
 *
 * @param TextToConvert [in] the string we want to convert to speech
  * @param QueueAudio [in] should audio be placed in a queue
 */
void ATtsExperience::ConvertTextToSpeech(const FString& TextToConvert, bool bQueueAudio)
{
	if (TtsService != nullptr)
	{
		InitializeService();
		TtsService->ConvertTextToSpeech(TextToConvert);
	}
}

/**
 * Sends a text string to Wit for conversion to speech with custom settings
 *
 * @param ClipSettings [in] the string we want to convert to speech
 * @param QueueAudio [in] should audio be placed in a queue
 */
void ATtsExperience::ConvertTextToSpeechWithSettings(const FTtsConfiguration& ClipSettings, bool bQueueAudio)
{
	if (TtsService != nullptr)
	{
		InitializeService();
		TtsService->ConvertTextToSpeechWithSettings(ClipSettings, bQueueAudio);
	}
}

/**
 * Fetch a list of available voices from Wit
 */
void ATtsExperience::FetchAvailableVoices()
{
	if (TtsService == nullptr)
	{
		return;
	}

	InitializeService();	
	TtsService->FetchAvailableVoices();
}

/**
 * Unload a single clip
 *
 * @param ClipId [in] Id of the clip to unload
 */
void ATtsExperience::UnloadClip(const FString& ClipId)
{
	if (MemoryCacheHandler != nullptr)
	{
		MemoryCacheHandler->RemoveClip(ClipId);
	}
}

/**
 * Unload all clips
 */
void ATtsExperience::UnloadAllClips()
{
	if (MemoryCacheHandler != nullptr)
	{
		MemoryCacheHandler->RemoveAllClips();
	}
}

/**
 * Remove a single clip from the storage cache
 *
 * @param ClipId [in] Id of the clip to delete
 * @param CacheLocation [in] location of the clip
 */
void ATtsExperience::DeleteClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation)
{
	UnloadClip(ClipId);
	
	if (StorageCacheHandler != nullptr)
	{
		StorageCacheHandler->RemoveClip(ClipId, CacheLocation);
	}
}

/**
 * Remove all clips from the storage cache
 *
 * @param CacheLocation [in] location of the clip
 */
void ATtsExperience::DeleteAllClips(const ETtsStorageCacheLocation CacheLocation)
{
	UnloadAllClips();
	
	if (StorageCacheHandler != nullptr)
	{
		StorageCacheHandler->RemoveAllClips(CacheLocation);
	}
}
