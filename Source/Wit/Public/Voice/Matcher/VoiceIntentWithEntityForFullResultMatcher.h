/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Voice/Matcher/VoiceIntentMatcher.h"
#include "Wit/Request/WitResponse.h"
#include "VoiceIntentWithEntityForFullResultMatcher.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnEntityMatchedForFullResultDelegate, FWitIntent, Intent, FWitEntities, AllEntities, TArray<FString>, Values, bool, bIsFinal);

/**
 * A simple response matcher. Examines a response from Wit.ai and checks to see if it contains the intent
 * and entities that we are looking for. If it does then we call a delegate for user-defined processing
 */
UCLASS(ClassGroup=(Meta), meta=(BlueprintSpawnableComponent))
class WIT_API UVoiceIntentWithEntityForFullResultMatcher final : public UVoiceResponseMatcher
{
	GENERATED_BODY()

public:
	
	/**
	 * Default constructor
	 */
	UVoiceIntentWithEntityForFullResultMatcher();
	
	/**
	 * Constructor that takes an intent name and a single entity name
	 * 
	 * @param IntentName [in] name of the intent to match against
	 * @param EntityName [in] name of the entity to match against
	 */
	explicit UVoiceIntentWithEntityForFullResultMatcher(const FString& IntentName, const FString& EntityName);

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
	 * The names of the entities to match 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Entity")
	FString RequiredEntityName{};

	/** 
	 * The confidence threshold to exceed to be considered a match
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Entity")
	float EntityConfidenceThreshold{0.5f};

	/**
	 * Delegate to call when this matcher makes a successful intent match but no entities match
	 */
	UPROPERTY(BlueprintAssignable)
	FOnIntentMatchedDelegate OnIntentMatched{};

	/**
	 * Delegate to call when this matcher makes a successful match for a single entity
	 *
	 * Wit.ai support multi results for each entity e.g. pick up apple, orange and banana.
	 *
	 * In that case the entity may have multi values, e,g apple, orange and banana.
	 * 
	 * UVoiceIntentWithEntityMatcher.OnEntityMatched only support one value, but OnEntityMatchedForFullResult support all values.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnEntityMatchedForFullResultDelegate OnEntityMatchedForFullResult{};

	/**
	 * Callback that is called when a Wit.ai response is received which is used to see if we have a match
	 * 
	 * @param bIsSuccessful [in] true if the response was successful
	 * @param Response [in] the full response as a UStruct
	 */
	virtual void OnWitResponse(const bool bIsSuccessful, const FWitResponse& Response) override;
	
};
