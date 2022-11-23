/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Wit/Request/WitResponse.h"
#include "UObject/NoExportTypes.h"
#include "Voice/Matcher/VoiceIntentMatcher.h"
#include "VoiceIntentWithEntitiesForFullResultMatcher.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnEntitiesMatchedForFullResultDelegate, bool, bIsSuccessful, FWitIntent, Intent, TArray<FWitEntities>, AllEntities, bool, bIsFinal); //TArray<TArray<FString>> Values cannot put here, Unreal doesn't support Nested Containers, so they have moved to each FWitEntities

/**
 * A simple response matcher. Examines a response from Wit.ai and checks to see if it contains the intent
 * and entities that we are looking for. If it does then we call a delegate for user-defined processing
 */
UCLASS(ClassGroup=(Meta), meta=(BlueprintSpawnableComponent))
class WIT_API UVoiceIntentWithEntitiesForFullResultMatcher final : public UVoiceResponseMatcher
{
	GENERATED_BODY()

public:
	
	/**
	 * Default constructor
	 */
	UVoiceIntentWithEntitiesForFullResultMatcher();
	
	/**
	 * Constructor that takes an intent name and a single entity name
	 * 
	 * @param IntentName [in] name of the intent to match against
	 */
	explicit UVoiceIntentWithEntitiesForFullResultMatcher(const FString& IntentName);
		
	/**
	 * The name of the intent to match 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Intent")
	FString IntentName{};

	/**
	 * Is the intent required to match?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Intent")
	bool bIsIntentRequired{true};

	/** 
	 * The confidence threshold to exceed to be considered a match
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Intent")
	float IntentConfidenceThreshold{0.5f};

	/**
	 * The names of the entities that must be matched
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Entities")
	TArray<FString> RequiredEntityNames{};

	/**
	 * The names of the entities that are optional matches
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Entities")
	TArray<FString> OptionalEntityNames{};

	/** 
	 * The confidence threshold to exceed to be considered a match
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Entities")
	float EntityConfidenceThreshold{0.5f};
	
	/**
	 * Delegate to call when this matcher makes a successful intent match but no entities match
	 */
	UPROPERTY(BlueprintAssignable)
	FOnIntentMatchedDelegate OnIntentMatched{};

	/**
	 * Delegate to call when this matcher makes a successful match against multiple entities
	 *
	 * Wit.ai support executing multi commands in one speech, e.g. set the box to red, and the cylinder to green.
	 *
	 * In that case each entity may have multi values, e,g red and green.
	 * 
	 * UVoiceIntentWithEntitiesMatcher.OnEntitiesMatched only support one value, but OnEntitiesMatchedForFullResult support all values.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnEntitiesMatchedForFullResultDelegate OnEntitiesMatchedForFullResult{};

	/**
	 * Callback that is called when a Wit.ai response is received which is used to see if we have a match
	 * 
	 * @param bIsSuccessful [in] true if the response was successful
	 * @param Response [in] the full response as a UStruct
	 */
	virtual void OnWitResponse(const bool bIsSuccessful, const FWitResponse& Response) override;

};
