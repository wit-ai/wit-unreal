/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Voice/WitVoiceExperience.h"
#include "CoreMinimal.h"
#include "Wit/Voice/WitVoiceService.h"

/**
 * Sets default values for this actor's properties
 */
AWitVoiceExperience::AWitVoiceExperience()
	: Super()
{	
	WitVoiceService = CreateDefaultSubobject<UWitVoiceService>(TEXT("WitVoiceService"));

	VoiceService = WitVoiceService;
}

/**
 * Choose which service to use
 */
void AWitVoiceExperience::BeginPlay()
{
	VoiceService = WitVoiceService;
	
	Super::BeginPlay();
}
