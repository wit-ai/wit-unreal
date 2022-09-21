/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Dictation/WitDictationExperience.h"
#include "CoreMinimal.h"
#include "Wit/Dictation/WitDictationService.h"

/**
 * Sets default values for this actor's properties
 */
AWitDictationExperience::AWitDictationExperience()
	: Super()
{	
	WitDictationService = CreateDefaultSubobject<UWitDictationService>(TEXT("WitDictationService"));

	DictationService = WitDictationService;
}

/**
 * Choose which service to use
 */
void AWitDictationExperience::BeginPlay()
{
	DictationService = WitDictationService;
	
	Super::BeginPlay();
}
