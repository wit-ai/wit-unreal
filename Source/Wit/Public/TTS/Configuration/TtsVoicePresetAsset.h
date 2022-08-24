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
#include "TtsVoicePresetAsset.generated.h"

/**
 * A simple data asset that stores a voice preset for text to speech
 */
UCLASS()
class WIT_API UTtsVoicePresetAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	//** Synthesize endpoint configuration */
	UPROPERTY(EditAnywhere, Category="TTS")
	FTtsConfiguration Synthesize;

};
