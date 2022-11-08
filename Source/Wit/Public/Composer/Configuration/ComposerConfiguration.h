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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer|Configuration")
	FName VoiceExperienceTag{};
	
	/** Should we redirect the voice service inputs and outputs to composer? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer|Configuration")
	bool bShouldRouteVoiceServiceToComposer{true};
	
	/** Should input be auto-activated when it is required? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer|Configuration")
	bool bShouldAutoActivateInput{true};

	/** Should the session be automatically ended when the composer graph ends? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer|Configuration")
	bool bShouldAutoEndSession{false};

	/** Should the context map be automatically cleared when the composer graph ends? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer|Configuration")
	bool bShouldAutoClearContextMap{false};

	/** Delay from action completion and response to listen for graph continuation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Composer|Configuration")
	float ContinueDelay{0.0f};
	
};
