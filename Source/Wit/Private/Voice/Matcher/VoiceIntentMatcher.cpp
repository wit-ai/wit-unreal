/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Voice/Matcher/VoiceIntentMatcher.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Default constructor
 */
UVoiceIntentMatcher::UVoiceIntentMatcher()
	: Super()
{
	// Deliberately empty
}

/**
 * Constructor that takes an intent name
 * 
 * @param IntentName [in] name of the intent to match against
 */
UVoiceIntentMatcher::UVoiceIntentMatcher(const FString& IntentName)
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
void UVoiceIntentMatcher::OnWitResponse(const bool bIsSuccessful, const FWitResponse& Response)
{
	if (!bIsSuccessful)
	{
		return;
	}

	const FWitIntent* MatchingIntent = FWitHelperUtilities::FindMatchingIntent(Response, IntentName, IntentConfidenceThreshold);

	const bool bHasMatchingIntent = (MatchingIntent != nullptr);
	if (!bHasMatchingIntent)
	{
		UE_LOG(LogWit, Verbose, TEXT("UVoiceIntentMatcher: intent does not match with (%s)"), *IntentName);		
		return;
	}

	OnIntentMatched.Broadcast(*MatchingIntent, Response.Is_Final);

	AcceptPartialResponse(Response);
}
