/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Voice/Matcher/VoiceIntentWithEntityForFullResultMatcher.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Default constructor
 */
UVoiceIntentWithEntityForFullResultMatcher::UVoiceIntentWithEntityForFullResultMatcher()
	: Super()
{
	// Deliberately empty
}

/**
 * Constructor that takes an intent name and a single entity name
 * 
 * @param IntentName [in] name of the intent to match against
 * @param EntityName [in] name of the entity to match against
 */
UVoiceIntentWithEntityForFullResultMatcher::UVoiceIntentWithEntityForFullResultMatcher(const FString& IntentName, const FString& EntityName)
	: Super(), IntentName(IntentName), RequiredEntityName(EntityName)
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
void UVoiceIntentWithEntityForFullResultMatcher::OnWitResponse(const bool bIsSuccessful, const FWitResponse& Response)
{
	if (!bIsSuccessful)
	{
		return;
	}

	// Try to match the intent first (if required)

	const FWitIntent* MatchingIntent = FWitHelperUtilities::FindMatchingIntent(Response, IntentName, IntentConfidenceThreshold);
	const bool bHasMatchingIntent = MatchingIntent != nullptr;

	if (bIsIntentRequired && !bHasMatchingIntent)
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntityForFullResultMatcher: intent does not match with (%s)"), *IntentName);
		return;
	}

	// Try to match the entity.
	
	FWitEntities FoundMatchingEntities{};
	const bool bIsFound = FWitHelperUtilities::FindMatchingEntities(Response, RequiredEntityName, EntityConfidenceThreshold ,FoundMatchingEntities);
	TArray<FString> Values{};

	// Call the relevant delegate
	
	if (!bIsFound)
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntityForFullResultMatcher: no entity successfully matched"));

		OnIntentMatched.Broadcast(MatchingIntent == nullptr ? FWitIntent() : *MatchingIntent, Response.Is_Final);
	}
	else
	{
		for (const FWitEntity& Entity : FoundMatchingEntities.Entities)
		{
			Values.Add(Entity.Value);
		}
		
		FoundMatchingEntities.Name = RequiredEntityName;
		
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntityForFullResultMatcher: single entity successfully matched"));

		OnEntityMatchedForFullResult.Broadcast(MatchingIntent == nullptr ? FWitIntent() : *MatchingIntent, FoundMatchingEntities, Values, Response.Is_Final);		

		AcceptPartialResponse(Response);
	}
}
