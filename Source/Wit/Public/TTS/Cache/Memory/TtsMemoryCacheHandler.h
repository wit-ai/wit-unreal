/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TTS/Cache/Memory/Interface/ITtsMemoryCacheHandlerBase.h"
#include "TTS/Configuration/TtsConfiguration.h"
#include "TtsMemoryCacheHandler.generated.h"

class USoundWave;

/**
 * Abstract class for implementing a memory cache. This should be the base if you want to implement your own memory cache implementation
 */
UCLASS(Abstract)
class WIT_API UTtsMemoryCacheHandler : public UActorComponent, public ITtsMemoryCacheHandlerBase
{
	GENERATED_BODY()

public:
	
	/**
	 * ITtsMemoryCacheHandler default implementation
	 */	
	virtual bool AddClip(const FString& ClipId, USoundWave* SoundWave, const FTtsConfiguration& ClipSettings) override { return false; }
	virtual bool RemoveClip(const FString& ClipId) override { return false; }
	virtual void RemoveAllClips() override {};
	virtual USoundWave* GetClip(const FString& ClipId) const override { return nullptr; }
	virtual TArray<USoundWave*> GetClips() const override { return TArray<USoundWave*>(); }
	
};