/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "TTS/Experience/TtsExperience.h"
#include "WitTtsExperience.generated.h"

/**
 * Top level actor for Wit TTS interactions
 */
UCLASS( ClassGroup=(Meta), Blueprintable )
class WIT_API AWitTtsExperience : public ATtsExperience
{
	GENERATED_BODY()

public:

	/**
	 * Sets default values for this actor's properties
	 */
	AWitTtsExperience();

};
