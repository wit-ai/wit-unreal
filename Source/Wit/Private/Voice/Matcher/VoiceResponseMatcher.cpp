/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Voice/Matcher/VoiceResponseMatcher.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Default constructor
 */
UVoiceResponseMatcher::UVoiceResponseMatcher()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = false;
}

/**
 * Called when play is started. Registers with the UWitVoiceService so we receive a callback when a new response is received
 */
void UVoiceResponseMatcher::BeginPlay()
{
	Super::BeginPlay();

	const AVoiceExperience* VoiceExperience = FWitHelperUtilities::FindVoiceExperience(GetWorld(), VoiceExperienceTag);
	
	if (VoiceExperience == nullptr)
	{
		return;
	}
	
	if (VoiceExperience->VoiceEvents == nullptr)
	{
		return;
	}

	UE_LOG(LogWit, Verbose, TEXT("UVoiceResponseMatcher: Registering response callback"));

	VoiceExperience->VoiceEvents->OnWitResponse.AddUniqueDynamic(this, &UVoiceResponseMatcher::OnWitResponse);
		
	if (bIsAlsoUsedForPartialResponse)
	{
		VoiceExperience->VoiceEvents->OnWitPartialResponse.AddUniqueDynamic(this, &UVoiceResponseMatcher::OnWitResponse);
	}
}

/**
 * Called when play is started. Registers with the UWitVoiceService so we receive a callback when a new response is received
 */
void UVoiceResponseMatcher::AcceptPartialResponse(const FWitResponse& Response)
{
	const bool bShouldProcessPartialResponse = !Response.Is_Final && bIsAlsoUsedForPartialResponse;
	
	if (!bShouldProcessPartialResponse)
	{
		return;
	}
	
	UE_LOG(LogWit, Verbose, TEXT("UVoiceResponseMatcher: the partial responhe partial response matched the intent and confidence threshold"));

	bool bShouldAcceptPartialResponse = false;
	
	if (bAutoAcceptPartialResponseOncePastMatcherCriteria)
	{
		UE_LOG(LogWit, Display, TEXT("UVoiceResponseMatcher: auto accept is on - accepting the partial response"));

		bShouldAcceptPartialResponse = true;
	}
	else if (PartialResponseValidator != nullptr)
	{
		if (PartialResponseValidatorInstance == nullptr)
		{
			PartialResponseValidatorInstance = NewObject<UWitPartialResponseValidatorBase>(this, PartialResponseValidator);
		}
		
		if (PartialResponseValidatorInstance != nullptr && IWitPartialResponseValidator::Execute_ValidatePartialResponse(PartialResponseValidatorInstance, Response))
		{
			UE_LOG(LogWit, Display, TEXT("UVoiceResponseMatcher: partial response validator is supplied and validated - accepting the partial response"));

			bShouldAcceptPartialResponse = true;
		}
	}

	if (bShouldAcceptPartialResponse)
	{
		FWitHelperUtilities::AcceptPartialResponseAndCancelRequest(this->GetWorld(), VoiceExperienceTag, Response);
	}
}
