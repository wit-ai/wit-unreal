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

/**
 * Composer session data used to provide easier access to all classes involved in a composer event
 */
USTRUCT(BlueprintType)
struct WIT_API FComposerSessionData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString SessionId;

private:
	
	TSharedPtr<FJsonObject> ContextMap;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComposerSessionEventDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComposerActiveEventDelegate);

/**
 * Container for all composer events
 */
UCLASS(ClassGroup=(Meta))
class WIT_API UComposerEvents final : public UActorComponent
{
	GENERATED_BODY()

public:
	
	/**
	 * Callback for composer session start
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer")
	FOnComposerSessionEventDelegate OnComposerSessionBegin{};

	/**
	 * Callback for composer session end
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer")
	FOnComposerSessionEventDelegate OnComposerSessionEnd{};

	/**
	 * Callback when active composer session changes
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer")
	FOnComposerActiveEventDelegate OnComposerSessionChange{};

	/**
	 * Callback when the context map is updated
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer")
	FOnComposerSessionEventDelegate OnComposerContextMapChange{};

	/**
	 * Callback when the service is activated
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer")
	FOnComposerSessionEventDelegate OnComposerActivation{};

	/**
	 * Callback for a composer response
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer")
	FOnComposerSessionEventDelegate OnComposerResponse{};
	
	/**
	 * Callback when a composer error happens
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer")
	FOnComposerSessionEventDelegate OnComposerError{};

	/**
	 * Callback for when a composer graph expects input
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer")
	FOnComposerSessionEventDelegate OnComposerExpectsInput{};

	/**
	 * Callback for when a composer graph will read a phrase
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer")
	FOnComposerSessionEventDelegate OnComposerSpeakPhrase{};

	/**
	 * Callback for when a composer graph should perform an action
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer")
	FOnComposerSessionEventDelegate OnComposerPerformAction{};

	/**
	 * Callback for when a composer graph is completed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer")
	FOnComposerSessionEventDelegate OnComposerComplete{};
	
};
