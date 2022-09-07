/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Composer/Handlers/Speech/ComposerSpeechDefaultHandler.h"

/**
 * Speaks the specified phrase
 *
 * @param Phrase [in] the text to speak
 * @param ContextMap [in] the context map to check
 */
void UComposerSpeechDefaultHandler::SpeakPhrase(const FString& Phrase, const TSharedPtr<FJsonObject> ContextMap)
{
	// TODO: Need to pass the context map here

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
bool UComposerSpeechDefaultHandler::IsSpeaking(const TSharedPtr<FJsonObject> ContextMap)
{
	const AWitTtsSpeaker* Speaker = GetSpeaker(ContextMap);

	if (Speaker == nullptr)
	{
		return false;
	}
	
	return Speaker->IsLoading() || Speaker->IsSpeaking();
}

/**
 * Get a speaker give their name
 *
 * @param ContextMap [in] the context map to check
 */
AWitTtsSpeaker* UComposerSpeechDefaultHandler::GetSpeaker(const TSharedPtr<FJsonObject> ContextMap) const
{
	const bool bIsNoSpeakers = Speakers.Num() == 0;
	
	if (bIsNoSpeakers)
	{
		return nullptr;
	}
	
	const FString SpeakerName = GetSpeakerName(ContextMap);	
	const bool bIsValidSpeakerName = !SpeakerName.IsEmpty();

	if (bIsValidSpeakerName)
	{
		for(const auto& Item : Speakers)
		{
			const bool bIsMatchingName = Item.SpeakerName.Equals(SpeakerName, ESearchCase::IgnoreCase);

			if (bIsMatchingName)
			{
				return Item.Speaker;
			}
		}

		return nullptr;
	}

	return Speakers[0].Speaker;
}

/**
 * Lookup the speaker name from the context map
 *
 * @param ContextMap [in] the context map to check
 */
FString UComposerSpeechDefaultHandler::GetSpeakerName(const TSharedPtr<FJsonObject> ContextMap) const
{
	const bool bIsSpeakerName = ContextMap != nullptr && ContextMap->HasField(SpeakerNameContextMapKey);

	if (bIsSpeakerName)
	{
		return ContextMap->GetStringField(SpeakerNameContextMapKey);
	}

	return FString();
}
