/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/TTS/WitTtsExperience.h"
#include "CoreMinimal.h"
#include "Wit/TTS/WitTtsService.h"

/**
 * Sets default values for this actor's properties
 */
AWitTtsExperience::AWitTtsExperience()
	: Super()
{
	TtsService = CreateDefaultSubobject<UWitTtsService>(TEXT("TtsService"));
}
