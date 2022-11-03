/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Dictation/MultiTranscription/DictationMultiTranscription.h"
#include "Wit/Utilities/WitHelperUtilities.h"

/**
 * Called when the component starts playing
 */
void UDictationMultiTranscription::BeginPlay()
{
	DictationExperience = FWitHelperUtilities::FindDictationExperience(GetWorld(), DictationExperienceTag);
	
	if (DictationExperience != nullptr && DictationExperience->DictationEvents != nullptr)
	{
		DictationExperience->DictationEvents->OnFullTranscription.AddUniqueDynamic(this, &UDictationMultiTranscription::OnFullTranscription);
		DictationExperience->DictationEvents->OnPartialTranscription.AddUniqueDynamic(this, &UDictationMultiTranscription::OnPartialTranscription);
	}

	Super::BeginPlay();
}

/**
 * Called when the component is destroyed
 */
void UDictationMultiTranscription::BeginDestroy()
{
	if (DictationExperience != nullptr && DictationExperience->DictationEvents != nullptr)
	{
		DictationExperience->DictationEvents->OnFullTranscription.RemoveDynamic(this, &UDictationMultiTranscription::OnFullTranscription);
		DictationExperience->DictationEvents->OnPartialTranscription.RemoveDynamic(this, &UDictationMultiTranscription::OnPartialTranscription);
	}
	
	Super::BeginDestroy();
}

/**
 * Clear the text and start again
 */
void UDictationMultiTranscription::Clear()
{
	Text.Empty();
	ActivationCount = 0;
	
	DoUpdateTranscription(FString());
}


/**
 * Callback when a full transcription is received
 */
void UDictationMultiTranscription::OnFullTranscription(const FString& FullTranscription)
{
	const bool bIsMaxActivationsReached = ActivationCount >= MaxActivations;
	++ActivationCount;
	
	if (bIsMaxActivationsReached)
	{
		ActivationCount = MaxActivations;

		if (!bAutoClearAfterMaxActivations)
		{
			return;
		}

		Clear();
	}
	
	AppendSeparator(Text);
	Text.Append(FullTranscription);

	DoUpdateTranscription(FString());	
}

/**
 * Callback when a partial transcription is received
 */
void UDictationMultiTranscription::OnPartialTranscription(const FString& PartialTranscription)
{
	DoUpdateTranscription(PartialTranscription);
}

/**
 * Actually update the transcription text
 */
void UDictationMultiTranscription::DoUpdateTranscription(const FString& PartialTranscription)
{
	const bool bIsMaxActivationsReached = ActivationCount >= MaxActivations;

	if (bIsMaxActivationsReached && bShouldUseLimit)
	{
		return;
	}
	
	const bool bIsPartialTranscription = PartialTranscription.Len() > 0;

	if (!bIsPartialTranscription)
	{
		OnTranscriptionUpdated.Broadcast(Text);
		return;
	}
	
	FString CurrentTranscription = Text;

	AppendSeparator(CurrentTranscription);
	CurrentTranscription.Append(PartialTranscription);

	OnTranscriptionUpdated.Broadcast(CurrentTranscription);
}

/**
 * Append the separator to the given string
 */
void UDictationMultiTranscription::AppendSeparator(FString& AppendTo)
{
	const bool bShouldAppendSeparator = AppendTo.Len() > 0;

	if (!bShouldAppendSeparator)
	{
		return;
	}

	for (int i = 0; i < LinesBetweenActivations; ++i)
	{
		AppendTo.AppendChar('\n');
	}

	AppendTo.Append(ActivationSeparator);
}
