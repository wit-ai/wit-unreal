/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "WitAppConfiguration.generated.h"

/**
 * Wit advanced application configuration
 */
USTRUCT(BlueprintType)
struct WIT_API FWitAppAdvancedConfiguration
{
	GENERATED_BODY()

	/**
	 * Specifies the base URL to use when making requests to Wit.ai. If left empty this will use the default base URL
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Request")
	FString URL{};
	
	/**
	 * The optional API version to use when making requests to Wit.ai
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Request")
	FString ApiVersion{};

	/**
	 * Should we use a custom HTTP request timeout?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Request")
	bool bIsCustomHttpTimeout{false};

	/**
	 * Custom request timeout in seconds
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Request", meta=(ClampMin = 1, ClampMax = 180))
	float HttpTimeout{180.0f};
	
};

/**
 * Wit general application configuration
 */
USTRUCT(BlueprintType)
struct WIT_API FWitAppConfiguration
{
	GENERATED_BODY()

	/**
	 * The client access token to use when calling Wit. This can be obtained from your App setup on Wit.ai
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Access Token")
	FString ClientAccessToken{};

	/**
	 * Whether to use platform integration or not
	 */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Integration")
	bool bIsPlatformIntegrationEnabled{false};
	
	/**
	 * Advanced configuration
	 */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	FWitAppAdvancedConfiguration Advanced{};

};
