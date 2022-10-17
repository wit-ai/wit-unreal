/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Voice/Matcher/VoiceIntentWithEntitiesMatcher.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Default constructor
 */
UVoiceIntentWithEntitiesMatcher::UVoiceIntentWithEntitiesMatcher()
	: Super()
{
	// Deliberately empty
}

/**
 * Constructor that takes an intent name and a single entity name
 * 
 * @param IntentName [in] name of the intent to match against
 */
UVoiceIntentWithEntitiesMatcher::UVoiceIntentWithEntitiesMatcher(const FString& IntentName)
	: Super(), IntentName(IntentName)
{
	// Deliberately empty
}

/**
 * Callback that is called when a Wit.ai response is received. Checks to see if the response matches what
 * we are looking for
 * 
 * @param bIsSuccessful [in] true if the response was successful
 * @param Response [in] the full response as a UStruct
 */
void UVoiceIntentWithEntitiesMatcher::OnWitResponse(const bool bIsSuccessful, const FWitResponse& Response)
{
	if (!bIsSuccessful)
	{
		return;
	}

	// Try to match the intent first

	const FWitIntent* MatchingIntent = FWitHelperUtilities::FindMatchingIntent(Response, IntentName, IntentConfidenceThreshold);
	const bool bHasMatchingIntent = MatchingIntent != nullptr;

	if (bIsIntentRequired && !bHasMatchingIntent)
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntitiesMatcher: intent does not match with (%s)"), *IntentName);
		return;
	}

	
	// Try to match all the required entities. We match in the order specified so that we guarantee the ordering when we
	// have multiple entities to match

	TArray<FWitEntity> MatchingEntities;
	int32 MatchingEntitiesNum = 0;
	TArray<FString> MatchingValues;

	for (const auto& EntityName : RequiredEntityNames)
	{
		const FWitEntity* MatchingEntity = FWitHelperUtilities::FindMatchingEntity(Response, EntityName, EntityConfidenceThreshold);

		if (MatchingEntity == nullptr)
		{
			// Add empty entries to make sure the array size is consistent with the inputs
			
			MatchingEntities.Add(FWitEntity());
			MatchingValues.Add(FString());
		}
		else
		{
			MatchingEntities.Add(*MatchingEntity);
			MatchingEntitiesNum++;
			MatchingValues.Add(MatchingEntity->Value);
		}
	}

	const bool bIsAllRequiredEntitiesMatched = MatchingEntitiesNum == RequiredEntityNames.Num();

	// Try to match all the optional entities. We match in the order specified so that we guarantee the ordering when we
	// have multiple entities to match

	for (const auto& EntityName : OptionalEntityNames)
	{
		const FWitEntity* MatchingEntity = FWitHelperUtilities::FindMatchingEntity(Response, EntityName, EntityConfidenceThreshold);

		if (MatchingEntity == nullptr)
		{
			// Add empty entries to make sure the array size is consistent with the inputs
			
			MatchingEntities.Add(FWitEntity());
			MatchingValues.Add(FString());
		}
		else
		{
			MatchingEntities.Add(*MatchingEntity);
			MatchingValues.Add(MatchingEntity->Value);
			MatchingEntitiesNum++;
		}
	}

	// Call the relevant delegate. We provide two variants just to make it more user friendly and convenient in the
	// common case of only needing to match the Intent

	const bool bIsNoEntityMatched = MatchingEntitiesNum == 0;
	
	if (bIsNoEntityMatched)
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntitiesMatcher: no entity successfully matched"));

		OnIntentMatched.Broadcast(MatchingIntent == nullptr ? FWitIntent() : *MatchingIntent, Response.Is_Final);
	}
	else
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntitiesMatcher: multiple entities successfully matched"));

		OnEntitiesMatched.Broadcast(bIsAllRequiredEntitiesMatched, MatchingIntent == nullptr ? FWitIntent() : *MatchingIntent, MatchingEntities, MatchingValues, Response.Is_Final);

		if (bIsAllRequiredEntitiesMatched)
		{
			AcceptPartialResponse(Response);
		}
	}
}
