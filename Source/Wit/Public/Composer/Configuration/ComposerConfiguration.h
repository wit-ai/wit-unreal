/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "ComposerConfiguration.generated.h"

/**
 * Composer configuration for Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FComposerConfiguration
{
	GENERATED_BODY()

	/** The tag of the voice experience to use for voice activation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer")
	FName VoiceExperienceTag{};
	
	/** Whether the composer service will be used for voice activation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer")
	bool bShouldRouteVoiceServiceToComposer{true};
	
	/** Whether this service should automatically handle input activation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer")
	bool bShouldExpectInputAutoActivation{true};

	/** Whether this service should automatically end the session graph completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer")
	bool bShouldEndSessionOnCompletion{false};

	/** Whether this service should automatically clear the context map on graph completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer")
	bool bShouldClearContextMapOnCompletion{false};

	/** Delay from action completion and response to listen for graph continuation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer")
	float ContinueDelay{0.0f};
	
};
