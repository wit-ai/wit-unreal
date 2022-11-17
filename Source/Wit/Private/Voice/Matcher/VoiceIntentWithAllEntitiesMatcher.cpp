/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Voice/Matcher/VoiceIntentWithAllEntitiesMatcher.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Default constructor
 */
UVoiceIntentWithAllEntitiesMatcher::UVoiceIntentWithAllEntitiesMatcher()
	: Super()
{
	// Deliberately empty
}

/**
 * Constructor that takes an intent name and a single entity name
 * 
 * @param IntentName [in] name of the intent to match against
 */
UVoiceIntentWithAllEntitiesMatcher::UVoiceIntentWithAllEntitiesMatcher(const FString& IntentName)
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
void UVoiceIntentWithAllEntitiesMatcher::OnWitResponse(const bool bIsSuccessful, const FWitResponse& Response)
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
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithAllEntitiesMatcher: intent does not match with (%s)"), *IntentName);
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
		TArray<FString> Values{};
		
		FWitEntities FoundMatchingEntities{};
		
		const bool bIsFound = FWitHelperUtilities::FindMatchingEntities(Response, EntityName, EntityConfidenceThreshold ,FoundMatchingEntities);

		if (!bIsFound)
		{
			// Add empty entries to make sure the array size is consistent with the inputs
			
			WitEntities.Entities.Add(FWitEntity());
			Values.Add(FString());
		}
		else
		{
			WitEntities.Entities.Append(FoundMatchingEntities.Entities);
			MatchingEntitiesNum++;
			for (const auto& MatchingEntity : FoundMatchingEntities.Entities)
			{
				Values.Add(MatchingEntity.Value);
			}
		}
		MatchingEntities.Add(WitEntities);
		MatchingValues.Add(Values);
	}

	const bool bIsAllRequiredEntitiesMatched = MatchingEntitiesNum == RequiredEntityNames.Num();

	// Try to match all the optional entities. We match in the order specified so that we guarantee the ordering when we
	// have multiple entities to match

	for (const auto& EntityName : OptionalEntityNames)
	{
		FWitEntities WitEntities{};
		WitEntities.Name = EntityName;
		TArray<FString> Values{};
		
		FWitEntities FoundMatchingEntities{};
		
		const bool bIsFound = FWitHelperUtilities::FindMatchingEntities(Response, EntityName, EntityConfidenceThreshold ,FoundMatchingEntities);

		if (!bIsFound)
		{
			// Add empty entries to make sure the array size is consistent with the inputs
			
			WitEntities.Entities.Add(FWitEntity());
			Values.Add(FString());
		}
		else
		{
			WitEntities.Entities.Append(FoundMatchingEntities.Entities);
			MatchingEntitiesNum++;
			for (const auto& MatchingEntity : FoundMatchingEntities.Entities)
			{
				Values.Add(MatchingEntity.Value);
			}
		}
		
		MatchingEntities.Add(WitEntities);
		MatchingValues.Add(Values);
	}

	// Call the relevant delegate. We provide two variants just to make it more user friendly and convenient in the
	// common case of only needing to match the Intent

	const bool bIsNoEntityMatched = MatchingEntitiesNum == 0;
	
	if (bIsNoEntityMatched)
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithAllEntitiesMatcher: no entity successfully matched"));

		OnIntentMatched.Broadcast(MatchingIntent == nullptr ? FWitIntent() : *MatchingIntent, Response.Is_Final);
	}
	else
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithAllEntitiesMatcher: multiple entities successfully matched"));

		OnEntityMatchedWithAllEntity.Broadcast(bIsAllRequiredEntitiesMatched, MatchingIntent == nullptr ? FWitIntent() : *MatchingIntent, MatchingEntities, Response.Is_Final);

		if (bIsAllRequiredEntitiesMatched)
		{
			AcceptPartialResponse(Response);
		}
	}
}
