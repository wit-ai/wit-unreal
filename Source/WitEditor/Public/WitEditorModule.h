/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"

/**
 * The module for the UE4 editor plugin implementation of the Wit API
 */
class FWitEditorModule final : public IModuleInterface
{
public:
	
	/**
	 * IModuleInterface implementation
	 */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
};
