/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "WitRequestConfiguration.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWitRequestErrorDelegate, const FString&, const FString&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWitRequestProgressDelegate, const TArray<uint8>&, const TSharedPtr<FJsonObject>);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWitRequestCompleteDelegate, const TArray<uint8>&, const TSharedPtr<FJsonObject>);

/**
 * A compact configuration for setting up a Wit.ai request. Use the methods in FWitRequestBuilder to construct this
 */
USTRUCT(BlueprintType)
struct WIT_API FWitRequestConfiguration
{
	GENERATED_BODY()

	/**
	 * Default constructor
	 */
	FWitRequestConfiguration() = default;

	/** The base URL to use in the request */
	FString BaseUrl{};

	/** Optional version modifier string to use in the request */
	FString Version{};

	/** The server auth token to use in the request. Must be present */
	FString ServerAuthToken{};

	/** The verb (POST/GET) to use in the request */
	FString Verb{};

	/** The endpoint to use in the request */
	FString Endpoint{};

	/** The expect to use in the request */
	FString Accept{};

	/** Optional set of URL parameters to use in the request */
	TMap<FString, FString> Parameters{};

	/** Optional content type pairs to use in the request */
	TMap<FString, FString> ContentTypes{};

	/** Optional callback to use when the request errors */
	FOnWitRequestErrorDelegate OnRequestError{};

	/** Optional callback to use when the request is in progress */
	FOnWitRequestProgressDelegate OnRequestProgress{};
	
	/** Optional callback to use when the request is complete */
	FOnWitRequestCompleteDelegate OnRequestComplete{};

	/** Tracks whether we should use the HTTP 1 chunked transfer protocol in the request */
	bool bShouldUseChunkedTransfer{false};
};
