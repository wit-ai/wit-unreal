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
	 * The configuration that will be used by composer
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer|Configuration")
	FComposerConfiguration Configuration{};
	
	/**
	 * The events used by the composer service
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composer|Handlers")
	UComposerEvents* EventHandler{};

	/**
	 * The action handler used by the composer service
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composer|Handlers")
	UComposerActionHandler* ActionHandler{};

	/**
	 * The speech handler user by the composer service
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composer|Handlers")
	UComposerSpeechHandler* SpeechHandler{};

	/**
	 * The underlying composer service that implements the composer functionality
	 */
	UPROPERTY()
	UWitComposerService* ComposerService{};

protected:

	virtual void BeginPlay() override;

};
