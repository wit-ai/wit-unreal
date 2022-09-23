/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Voice/Experience/VoiceExperience.h"
#include "Wit/Voice/WitVoiceService.h"
#include "WitVoiceExperience.generated.h"

/**
 * Top level actor for Wit voice interactions. You can create a blueprint class off this actor and add whatever intent and entity matchers
 * you need to implement your voice experience
 */
UCLASS( ClassGroup=(Meta), Blueprintable)
class WIT_API AWitVoiceExperience final : public AVoiceExperience
{
	GENERATED_BODY()
	
public:

	/**
	 * Sets default values for this actor's properties
	 */
	AWitVoiceExperience();

	/**
	 * The wit voice service
	 */
	UPROPERTY()
	UWitVoiceService* WitVoiceService{};

protected:

	virtual void BeginPlay() override;

};
