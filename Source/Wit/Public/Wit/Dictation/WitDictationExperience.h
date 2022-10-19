/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "WitDictationService.h"
#include "Dictation/Experience/DictationExperience.h"
#include "WitDictationExperience.generated.h"

/**
 * Top level actor for Wit dictation interactions
 */
UCLASS( ClassGroup=(Meta), Blueprintable )
class WIT_API AWitDictationExperience final : public ADictationExperience
{
	GENERATED_BODY()

public:

	/**
	 * Sets default values for this actor's properties
	 */
	AWitDictationExperience();
	
	/**
	 * The wit dictation service
	 */
	UPROPERTY()
	UWitDictationService* WitDictationService{};
	
protected:

	virtual void BeginPlay() override;
	
};
