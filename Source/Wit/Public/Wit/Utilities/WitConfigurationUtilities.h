/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "Wit/Configuration/WitAppConfigurationAsset.h"
#include "Wit/Request/WitRequestConfiguration.h"
#include "Wit/Request/WitRequestTypes.h"

class WIT_API FWitConfigurationUtilities
{
public:

	/** Refresh all the associated app data for a configuration */
	static void RefreshConfiguration(UWitAppConfigurationAsset* ConfigurationToRefresh);
	
	/** Is a request currently in progress? */
	static bool IsRefreshInProgress();

private:

	/** Current configuration we are acting on */
	static UWitAppConfigurationAsset* Configuration;

	/** Request the list of apps for a user */
	static void RequestAppList();

	/** Request the client token for an app */
	static void RequestClientToken(const FString& AppId);

	/** Request the list of intents/entities/traits for an app */
	static void RequestIntentList();
	static void RequestEntityList();
	static void RequestTraitList();

	/** Request the list of voices for an app */
	static void RequestVoiceList();

	/** Called when a Wit apps request is completed */
	static void OnAppsRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse);
	static void OnAppsRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage);

	/** Called when a Wit client token request is completed */
	static void OnClientTokenRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse);
	static void OnClientTokenRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage);

	/** Called when a Wit intents request is completed */
	static void OnIntentsRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse);
	static void OnIntentsRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage);

	/** Called when a Wit entities request is completed */
	static void OnEntitiesRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse);
	static void OnEntitiesRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage);

	/** Called when a Wit traits request is completed */
	static void OnTraitsRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse);
	static void OnTraitsRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage);

	/** Called when a Wit voices request is completed */
	static void OnVoicesRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse);
	static void OnVoicesRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage);

	/** Helper for making list requests */
	static bool SetupListRequest(FWitRequestConfiguration& RequestConfiguration, EWitRequestEndpoint Endpoint, const bool bIsServerAuthRequired);

};
