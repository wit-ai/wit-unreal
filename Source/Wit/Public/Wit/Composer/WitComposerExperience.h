/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "WitComposerService.h"
#include "Composer/Configuration/ComposerConfiguration.h"
#include "Composer/Events/ComposerEvents.h"
#include "Wit/Voice/WitVoiceExperience.h"
#include "WitComposerExperience.generated.h"

/**
 * Top level actor for Wit composer interactions
 */
UCLASS( ClassGroup=(Meta) )
class WIT_API AWitComposerExperience final : public AActor
{
	GENERATED_BODY()

public:

	/**
	 * Sets default values for this actor's properties
	 */
	AWitComposerExperience();

	/**
	 * The configuration that will be used 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer")
	FComposerConfiguration Configuration{};

	/**
	 * The underlying composer service that implements the composer functionality
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Composer")
	UWitComposerService* ComposerService{};
	
	/**
	 * The events used by the composer service
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Composer")
	UComposerEvents* EventHandler{};

	/**
	 * The action handler
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Composer")
	UComposerActionHandler* ActionHandler{};

	/**
	 * The speech handler
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Composer")
	UComposerSpeechHandler* SpeechHandler{};

protected:

	virtual void BeginPlay() override;

};
