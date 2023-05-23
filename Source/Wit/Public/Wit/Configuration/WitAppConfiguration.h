/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "Wit/Request/WitResponse.h"

#include "WitAppConfiguration.generated.h"

/**
 * Wit advanced application configuration
 */
USTRUCT(BlueprintType)
struct WIT_API FWitAppAdvancedConfiguration
{
	GENERATED_BODY()

	/** Should we use platform integration? */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform Integration")
	bool bIsPlatformIntegrationEnabled{false};

	/** Specifies the base URL to use when making requests to Wit.ai. If left empty this will use the default base URL */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Request")
	FString URL{};
	
	/** The optional API version to use when making requests to Wit.ai */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Request Overrides")
	FString ApiVersion{"20230523"};

	/** Should we use a custom HTTP request timeout? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Request Overrides")
	bool bIsCustomHttpTimeout{false};

	/** Custom request timeout in seconds. This is only used if bIsCustomHttpTimeout is set to true */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Request Overrides", meta=(ClampMin = 1, ClampMax = 180))
	float HttpTimeout{180.0f};
	
};

/**
 * Wit application data configuration. This contains all the data such as entities and intents that have been defined for the application
 */
USTRUCT(BlueprintType)
struct WIT_API FWitAppDataConfiguration
{
	GENERATED_BODY()
		
	/** Basic application data */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Application")
	FWitAppDefinition Application;

	/** List of intents available in the app */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Intents")
	TArray<FWitIntentDefinition> Intents;
	
	/** List of entities available in the app */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entities")
	TArray<FWitEntityDefinition> Entities;

	/** List of traits available in the app */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traits")
	TArray<FWitTraitDefinition> Traits;

	/** List of voices available in the app */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voices")
	TArray<FWitVoiceDefinition> Voices;
	
};


/**
 * Wit general application configuration
 */
USTRUCT(BlueprintType)
struct WIT_API FWitAppConfiguration
{
	GENERATED_BODY()

	/** The server access token to use when calling Wit. This can be obtained from your App setup on Wit.ai */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Token")
	FString ServerAccessToken{};
	
	/** The client access token to use when calling Wit. This can be obtained from your App setup on Wit.ai */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Token")
	FString ClientAccessToken{};
	
	/** Advanced configuration */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	FWitAppAdvancedConfiguration Advanced{};

	/** Data configuration */	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	FWitAppDataConfiguration Data{};

};
