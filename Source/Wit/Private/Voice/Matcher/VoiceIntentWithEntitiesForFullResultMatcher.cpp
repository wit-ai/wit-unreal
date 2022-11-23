/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Voice/Matcher/VoiceIntentWithEntitiesForFullResultMatcher.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Default constructor
 */
UVoiceIntentWithEntitiesForFullResultMatcher::UVoiceIntentWithEntitiesForFullResultMatcher()
	: Super()
{
	// Deliberately empty
}

/**
 * Constructor that takes an intent name and a single entity name
 * 
 * @param IntentName [in] name of the intent to match against
 */
UVoiceIntentWithEntitiesForFullResultMatcher::UVoiceIntentWithEntitiesForFullResultMatcher(const FString& IntentName)
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
void UVoiceIntentWithEntitiesForFullResultMatcher::OnWitResponse(const bool bIsSuccessful, const FWitResponse& Response)
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
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntitiesForFullResultMatcher: intent does not match with (%s)"), *IntentName);
		return;
	}

	
	// Try to match all the required entities. We match in the order specified so that we guarantee the ordering when we
	// have multiple entities to match

	TArray<FWitEntities> MatchingEntities;
	int32 MatchingEntitiesNum = 0;
	TArray<TArray<FString>> MatchingValues;

	for (const auto& EntityName : RequiredEntityNames)
	{
		FWitEntities WitEntities{};
		WitEntities.Name = EntityName;
		
		FWitEntities FoundMatchingEntities{};
		const bool bIsFound = FWitHelperUtilities::FindMatchingEntities(Response, EntityName, EntityConfidenceThreshold ,FoundMatchingEntities);

		if (!bIsFound)
		{
			// Add empty entries to make sure the array size is consistent with the inputs
			
			WitEntities.Entities.Add(FWitEntity());
		}
		else
		{
			WitEntities.Entities.Append(FoundMatchingEntities.Entities);
			MatchingEntitiesNum++;
		}
		MatchingEntities.Add(WitEntities);
	}

	const bool bIsAllRequiredEntitiesMatched = MatchingEntitiesNum == RequiredEntityNames.Num();

	// Try to match all the optional entities. We match in the order specified so that we guarantee the ordering when we
	// have multiple entities to match

	for (const auto& EntityName : OptionalEntityNames)
	{
		FWitEntities WitEntities{};
		WitEntities.Name = EntityName;
		
		FWitEntities FoundMatchingEntities{};
		const bool bIsFound = FWitHelperUtilities::FindMatchingEntities(Response, EntityName, EntityConfidenceThreshold ,FoundMatchingEntities);

		if (!bIsFound)
		{
			// Add empty entries to make sure the array size is consistent with the inputs
			
			WitEntities.Entities.Add(FWitEntity());
		}
		else
		{
			WitEntities.Entities.Append(FoundMatchingEntities.Entities);
			MatchingEntitiesNum++;
		}
		
		MatchingEntities.Add(WitEntities);
	}

	// Call the relevant delegate. We provide two variants just to make it more user friendly and convenient in the
	// common case of only needing to match the Intent

	const bool bIsNoEntityMatched = MatchingEntitiesNum == 0;
	
	if (bIsNoEntityMatched)
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntitiesForFullResultMatcher: no entity successfully matched"));

		OnIntentMatched.Broadcast(MatchingIntent == nullptr ? FWitIntent() : *MatchingIntent, Response.Is_Final);
	}
	else
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntitiesForFullResultMatcher: multiple entities successfully matched"));

		OnEntitiesMatchedForFullResult.Broadcast(bIsAllRequiredEntitiesMatched, MatchingIntent == nullptr ? FWitIntent() : *MatchingIntent, MatchingEntities, Response.Is_Final);

		if (bIsAllRequiredEntitiesMatched)
		{
			AcceptPartialResponse(Response);
		}
	}
}
