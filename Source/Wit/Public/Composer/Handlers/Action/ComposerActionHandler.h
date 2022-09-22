/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/IComposerActionHandlerBase.h"
#include "ComposerActionHandler.generated.h"

/**
 * Abstract class for implementing a composer action handler. This should be the base if you want to implement your own action handler implementation
 */
UCLASS(NotBlueprintable, Abstract)
class WIT_API UComposerActionHandler : public UActorComponent, public IComposerActionHandlerBase
{
	GENERATED_BODY()

public:
	
	/**
	 * IComposerActionHandlerBase default implementation
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer")
	virtual void PerformAction(const FString& Action, UComposerContextMap* ContextMap) override {}

	UFUNCTION(BlueprintCallable, Category = "Composer")
	virtual bool IsPerformingAction(const FString& Action) override { return false; }

	UFUNCTION(BlueprintCallable, Category = "Composer")
	virtual void MarkActionComplete(const FString& Action) override {}
	
};
