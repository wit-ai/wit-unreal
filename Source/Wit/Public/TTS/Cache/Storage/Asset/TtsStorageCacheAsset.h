/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TTS/Configuration/TtsConfiguration.h"
#include "TtsStorageCacheAsset.generated.h"

/**
 * A simple data asset that stores the binary representation of a clip
 */
UCLASS()
class WIT_API UTtsStorageCacheAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	/** The settings that were used to generate the clip */
	UPROPERTY(VisibleAnywhere, Category="TTS")
	FTtsConfiguration ClipSettings;
	
	/** The data of the clip */
	UPROPERTY(VisibleAnywhere, Category="TTS")
	TArray<uint8> ClipData; 
};
