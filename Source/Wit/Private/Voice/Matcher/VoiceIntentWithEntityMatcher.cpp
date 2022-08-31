/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Voice/Matcher/VoiceIntentWithEntityMatcher.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Default constructor
 */
UVoiceIntentWithEntityMatcher::UVoiceIntentWithEntityMatcher()
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
UVoiceIntentWithEntityMatcher::UVoiceIntentWithEntityMatcher(const FString& IntentName, const FString& EntityName)
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
void UVoiceIntentWithEntityMatcher::OnWitResponse(const bool bIsSuccessful, const FWitResponse& Response)
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
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntityMatcher: intent does not match with (%s)"), *IntentName);
		return;
	}

	// Try to match the entity.
	
	const FWitEntity* MatchingEntity = FWitHelperUtilities::FindMatchingEntity(Response, RequiredEntityName, EntityConfidenceThreshold);
	const bool bHasMatchingEntity = MatchingEntity != nullptr;

	// Call the relevant delegate
	
	if (!bHasMatchingEntity)
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntityMatcher: no entity successfully matched"));

		OnIntentMatched.Broadcast(MatchingIntent == nullptr ? FWitIntent() : *MatchingIntent, Response.Is_Final);
	}
	else
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentWithEntityMatcher: single entity successfully matched"));

		OnEntityMatched.Broadcast(MatchingIntent == nullptr ? FWitIntent() : *MatchingIntent, *MatchingEntity, MatchingEntity->Value, Response.Is_Final);		

		AcceptPartialResponse(Response);
	}
}
