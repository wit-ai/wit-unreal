/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "DictationConfiguration.generated.h"

/**
 * Dictation configuration for Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FDictationConfiguration
{
	GENERATED_BODY()

	/** The tag of the voice experience to use for voice activation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer|Configuration")
	FName VoiceExperienceTag{};
	
};
