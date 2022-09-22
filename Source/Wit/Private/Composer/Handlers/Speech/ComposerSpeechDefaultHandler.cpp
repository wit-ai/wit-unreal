/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Composer/Handlers/Speech/ComposerSpeechDefaultHandler.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Speaks the specified phrase
 *
 * @param Phrase [in] the text to speak
 * @param ContextMap [in] the context map to check
 */
void UComposerSpeechDefaultHandler::SpeakPhrase(const FString& Phrase, const UComposerContextMap* ContextMap)
{
	AWitTtsSpeaker* Speaker = GetSpeaker(ContextMap);

	if (Speaker == nullptr)
	{
		return;
	}

	Speaker->Speak(Phrase);
}

/**
 * Checks to see if we are in the middle of speaking a given phrase
 *
 * @param ContextMap [in] the context map to check
 *
 * @return true if we are currently speaking
 */
bool UComposerSpeechDefaultHandler::IsSpeaking(const UComposerContextMap* ContextMap) const
{
	const AWitTtsSpeaker* Speaker = GetSpeaker(ContextMap);

	if (Speaker == nullptr)
	{
		return false;
	}

	UE_LOG(LogWit, VeryVerbose, TEXT("IsSpeaking: is loading (%d) is speaking (%d)"), Speaker->IsLoading(), Speaker->IsSpeaking());
	
	return Speaker->IsLoading() || Speaker->IsSpeaking();
}

/**
 * Get a speaker give their name
 *
 * @param ContextMap [in] the context map to check
 */
AWitTtsSpeaker* UComposerSpeechDefaultHandler::GetSpeaker(const UComposerContextMap* ContextMap) const
{
	const bool bIsSpeakerAvailable = Speakers.Num() > 0;
	
	if (!bIsSpeakerAvailable)
	{
		return nullptr;
	}
	
	const FString SpeakerName = GetSpeakerName(ContextMap);	
	const bool bIsSpecificSpeakerRequested = !SpeakerName.IsEmpty();

	// If there is not specific speaker requested then we just pick the first one available
	
	if (!bIsSpecificSpeakerRequested)
	{
		return Speakers[0].Speaker;
	}

	// Otherwise look for the specific speaker that was asked for
	
	for(const auto& Item : Speakers)
	{
		const bool bIsMatchingName = Item.SpeakerName.Equals(SpeakerName, ESearchCase::IgnoreCase);

		if (bIsMatchingName)
		{
			return Item.Speaker;
		}
	}

	// This follows Unity behaviour where if a specific speaker is asked for but not found then we don't return any speaker (as opposed to the
	// other option of returning the first available)
	
	return nullptr;
}

/**
 * Lookup the speaker name from the context map
 *
 * @param ContextMap [in] the context map to check
 */
FString UComposerSpeechDefaultHandler::GetSpeakerName(const UComposerContextMap* ContextMap) const
{
	const bool bIsSpeakerName = ContextMap != nullptr && ContextMap->HasField(SpeakerNameContextMapKey);

	if (bIsSpeakerName)
	{
		FString SpeakerName;
		
		ContextMap->GetStringField(SpeakerNameContextMapKey, SpeakerName);
	}

	return FString();
}
