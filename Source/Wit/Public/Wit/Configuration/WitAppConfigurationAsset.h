/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Wit/Configuration/WitAppConfiguration.h"
#include "Engine/DataAsset.h"
#include "Voice/Configuration/VoiceConfiguration.h"
#include "WitAppConfigurationAsset.generated.h"

/**
 * A simple data asset that stores the binary representation of a clip
 */
UCLASS()
class WIT_API UWitAppConfigurationAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	
	/** The general application configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FWitAppConfiguration Application;
	
	//** Speech endpoint configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FVoiceConfiguration Voice;
	
};
