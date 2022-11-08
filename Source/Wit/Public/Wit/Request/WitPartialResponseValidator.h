/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "WitResponse.h"
#include "UObject/Interface.h"
#include "WitPartialResponseValidator.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UWitPartialResponseValidator : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WIT_API IWitPartialResponseValidator
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	
public:
	
	UFUNCTION(BlueprintnativeEvent, BlueprintCallable, Category="Validator")
	bool ValidatePartialResponse (const FWitResponse& Response);
};
