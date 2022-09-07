/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Composer/WitComposerExperience.h"
#include "CoreMinimal.h"
#include "Composer/Handlers/Action/ComposerActionDefaultHandler.h"
#include "Composer/Handlers/Speech/ComposerSpeechDefaultHandler.h"
#include "Wit/Composer/WitComposerService.h"

/**
 * Sets default values for this actor's properties
 */
AWitComposerExperience::AWitComposerExperience()
	: Super()
{	
	ComposerService = CreateDefaultSubobject<UWitComposerService>(TEXT("ComposerService"));

	// Create handlers
	
	EventHandler = CreateDefaultSubobject<UComposerEvents>(TEXT("EventHandler"));
	ActionHandler = CreateDefaultSubobject<UComposerActionDefaultHandler>(TEXT("ActionHandler"));
	SpeechHandler = CreateDefaultSubobject<UComposerSpeechDefaultHandler>(TEXT("SpeechHandler"));
}

/**
 * Called when play begins
 */
void AWitComposerExperience::BeginPlay()
{
	if (ComposerService != nullptr)
	{
		ComposerService->SetHandlers(EventHandler, ActionHandler, SpeechHandler);
		ComposerService->SetConfiguration(&Configuration);
	}
	
	Super::BeginPlay();
}
