/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "WitPartialResponseValidator.h"
#include "UObject/Object.h"
#include "WitPartialResponseValidatorBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class WIT_API UWitPartialResponseValidatorBase : public UObject, public IWitPartialResponseValidator
{
	GENERATED_BODY()
};
