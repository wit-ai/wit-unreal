/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TTS/Cache/Storage/Interface/ITtsStorageCacheHandlerBase.h"
#include "TTS/Configuration/TtsConfiguration.h"
#include "TTSStorageCacheHandler.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRequestClipCompleteDelegate, const bool, bIsSuccessful, TArray<uint8>, RawData);

/**
 * Abstract class for implementing a storage cache. This should be the base if you want to implement your own storage cache implementation
 */
UCLASS(Abstract)
class WIT_API UTtsStorageCacheHandler : public UActorComponent, public ITtsStorageCacheHandlerBase
{
	GENERATED_BODY()

public:
	
	/**
	 * IWitStorageCacheHandler default implementation
	 */	
	virtual bool GetCachePath(const ETtsStorageCacheLocation CacheLocation, FString& CachePath) const override { return false; };
	virtual bool ShouldCache(const ETtsStorageCacheLocation CacheLocation) const override { return false; };
	virtual bool AddClip(const FString& ClipId, const TArray<uint8>& ClipData, const FTtsConfiguration& ClipSettings) override { return false; };
	virtual bool RequestClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation, TArray<uint8>& ClipData) const override { return false; };
	virtual bool RemoveClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation) override { return false; };
	virtual void RemoveAllClips(const ETtsStorageCacheLocation CacheLocation) override {};
	
};
