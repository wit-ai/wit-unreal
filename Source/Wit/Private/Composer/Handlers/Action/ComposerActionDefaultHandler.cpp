/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Composer/Handlers/Action/ComposerActionDefaultHandler.h"

/**
 * Performs an action with the given id
 *
 * @param Action [in] the id of the action to perform
 */
void UComposerActionDefaultHandler::PerformAction(const FString& Action, UComposerContextMap* ContextMap)
{
	if (IsPerformingAction(Action))
	{
		return;
	}

	if (!OnPerformAction.IsBound())
	{
		return;
	}
	
	ActionsInProgress.Add(Action);

	OnPerformAction.Broadcast(Action, ContextMap);
}

/**
 * Checks to see if we are in the middle of performing a given action
 *
 * @param Action [in] the id of the action we want to query
 *
 * @return true if the action is in progress
 */
bool UComposerActionDefaultHandler::IsPerformingAction(const FString& Action)
{
	return ActionsInProgress.Contains(Action);
}

/**
 * Mark an action as complete
 *
 * @param Action [in] the id of the action to perform
 */
void UComposerActionDefaultHandler::MarkActionComplete(const FString& Action)
{
	ActionsInProgress.Remove(Action);
}
