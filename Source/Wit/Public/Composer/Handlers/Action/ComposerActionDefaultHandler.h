/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "ComposerActionHandler.h"
#include "ComposerActionDefaultHandler.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComposerActionDelegate, const FString&, Action, UComposerContextMap*, ContextMap);

/**
 * Default class for implementing a composer action handler. 
 */
UCLASS(Blueprintable, ClassGroup=(Meta), meta=(BlueprintSpawnableComponent))
class WIT_API UComposerActionDefaultHandler : public UComposerActionHandler
{
	GENERATED_BODY()

public:
	
	/**
	 * IComposerActionHandlerBase overrides
	 */
	virtual void PerformAction(const FString& Action, UComposerContextMap* ContextMap) override;
	virtual bool IsPerformingAction(const FString& Action) override;
	virtual void MarkActionComplete(const FString& Action) override;

	/**
	 * Blueprint event that can be implemented to perform the action
	 */
	UPROPERTY(BlueprintAssignable, Category = "Composer|Action")
	FOnComposerActionDelegate OnPerformAction{};


private:

	/** All the actions that are currently being performed */
	TSet<FString> ActionsInProgress{};
	
};
