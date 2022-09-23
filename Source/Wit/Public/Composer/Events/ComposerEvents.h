/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComposerEvents.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComposerSessionEventDelegate, UComposerContextMap*, ContextMap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComposerActiveEventDelegate);

/**
 * Container for all composer events
 */
UCLASS(NotBlueprintable)
class WIT_API UComposerEvents final : public UActorComponent
{
	GENERATED_BODY()

public:
	
	/**
	 * Callback for composer session start
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Events")
	FOnComposerSessionEventDelegate OnComposerSessionBegin{};

	/**
	 * Callback for composer session end
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Events")
	FOnComposerSessionEventDelegate OnComposerSessionEnd{};

	/**
	 * Callback when active composer session changes
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Events")
	FOnComposerActiveEventDelegate OnComposerSessionChange{};

	/**
	 * Callback when the context map is updated
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Events")
	FOnComposerSessionEventDelegate OnComposerContextMapChange{};

	/**
	 * Callback when the service is activated
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Events")
	FOnComposerSessionEventDelegate OnComposerActivation{};

	/**
	 * Callback for a composer response
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Events")
	FOnComposerSessionEventDelegate OnComposerResponse{};
	
	/**
	 * Callback when a composer error happens
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Events")
	FOnComposerSessionEventDelegate OnComposerError{};

	/**
	 * Callback for when a composer graph expects input
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Events")
	FOnComposerSessionEventDelegate OnComposerExpectsInput{};

	/**
	 * Callback for when a composer graph will read a phrase
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Events")
	FOnComposerSessionEventDelegate OnComposerSpeakPhrase{};

	/**
	 * Callback for when a composer graph should perform an action
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Events")
	FOnComposerSessionEventDelegate OnComposerPerformAction{};

	/**
	 * Callback for when a composer graph is completed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Events")
	FOnComposerSessionEventDelegate OnComposerComplete{};
	
};
