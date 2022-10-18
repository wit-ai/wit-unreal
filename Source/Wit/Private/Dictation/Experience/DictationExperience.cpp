/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Dictation/Experience/DictationExperience.h"
#include "CoreMinimal.h"

/**
 * Sets default values for this actor's properties
 */
ADictationExperience::ADictationExperience()
	: Super()
{
	DictationEvents = CreateDefaultSubobject<UDictationEvents>(TEXT("DictationEvents"));
}

/**
 * Called when the actor starts playing
 */
void ADictationExperience::BeginPlay()
{
	if (DictationService != nullptr)
	{
		DictationService->SetConfiguration(&Configuration);
		DictationService->SetEvents(DictationEvents);
	}
	
	Super::BeginPlay();
}

/**
 * Starts receiving dictation from the microphone and begins streaming it to for interpretation
 *
 * @return true if the activation was successful
 */
bool ADictationExperience::ActivateDictation()
{
	if (DictationService != nullptr)
	{
		return DictationService->ActivateDictation();
	}

	return false;
}

/**
 * Starts receiving dictation from the microphone and begins streaming it to VoiceService for interpretation
 *
 * @param RequestOptions [in] the RequestOptions string, this could be used for Dynamic Entities
 * 
 * @return true if the activation was successful
 */
bool ADictationExperience::ActivateDictationWithRequestOptions(const FString& RequestOptions)
{
	if (DictationService != nullptr)
	{
		return DictationService->ActivateDictationWithRequestOptions(RequestOptions);
	}

	return false;
}


/**
 * Starts receiving dictation from the microphone and begins streaming it to for interpretation immediately
 *
 * @return true if the activation was successful
 */
bool ADictationExperience::ActivateDictationImmediately()
{
	if (DictationService != nullptr)
	{
		return DictationService->ActivateDictationImmediately();
	}
	
	return false;
}

/**
 * Stops receiving dictation from the microphone and stops streaming it
 *
 * @return true if the deactivation was successful
 */
bool ADictationExperience::DeactivateDictation()
{
	if (DictationService != nullptr)
	{
		return DictationService->DeactivateDictation();
	}
	
	return false;
}

/**
 * Is dictation active on this component?
 *
 * @return true if active otherwise false
 */
bool ADictationExperience::IsDictationActive() const
{
	if (DictationService != nullptr)
	{
		return DictationService->IsDictationActive();
	}

	return false;
}

/**
 * Is any request currently in progress?
 *
 * @return true if in progress otherwise false
 */
bool ADictationExperience::IsRequestInProgress() const
{
	if (DictationService != nullptr)
	{
		return DictationService->IsRequestInProgress();
	}
	
	return false;
}
