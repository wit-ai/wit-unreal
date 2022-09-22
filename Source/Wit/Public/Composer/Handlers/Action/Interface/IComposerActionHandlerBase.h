/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Composer/Data/ComposerContextMap.h"

/**
 * Interface for implementing actions in a composer graph
 */
class IComposerActionHandlerBase
{
public:

	/** 
	 * Destructor for overrides 
	 */
	virtual ~IComposerActionHandlerBase() = default;

	/**
	 * Performs an action with the given id
	 *
	 * @param Action [in] the id of the action to perform
	 */
	virtual void PerformAction(const FString& Action, UComposerContextMap* ContextMap) = 0;

	/**
	 * Checks to see if we are in the middle of performing a given action
	 *
	 * @param Action [in] the id of the action we want to query
	 *
	 * @return true if the action is in progress
	 */
	virtual bool IsPerformingAction(const FString& Action) = 0;

	/**
	 * Marks an action as being finished
	 *
	 * @param Action [in] the id of the action to mark complete
	 */
	virtual void MarkActionComplete(const FString& Action) = 0;

};
