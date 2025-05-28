/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Voice/Experience/VoiceExperience.h"
#include "CoreMinimal.h"

/**
 * Sets default values for this actor's properties
 */
AVoiceExperience::AVoiceExperience()
	: Super()
{
	PrimaryActorTick.bCanEverTick = true;
	
	VoiceEvents = CreateDefaultSubobject<UVoiceEvents>(TEXT("VoiceEvents"));
}

/**
 * Called when the actor starts playing
 */
void AVoiceExperience::BeginPlay()
{
	if (VoiceService != nullptr)
	{
		VoiceService->SetEvents(VoiceEvents);
		VoiceService->SetConfiguration(Configuration, bUseWebSocket);
	}
	
	Super::BeginPlay();
}

/**
 * Starts receiving voice input from the microphone and begins streaming it to for interpretation
 *
 * @return true if the activation was successful
 */
bool AVoiceExperience::ActivateVoiceInput()
{
	if (VoiceService != nullptr)
	{
		return VoiceService->ActivateVoiceInput();
	}

	return false;
}

/**
 * Starts receiving voice input from the microphone and begins streaming it to VoiceService for interpretation
 *
 * @param RequestOptions [in] the RequestOptions string, this could be used for Dynamic Entities
 * 
 * @return true if the activation was successful
 */
bool AVoiceExperience::ActivateVoiceInputWithRequestOptions(const FString& RequestOptions)
{
	if (VoiceService != nullptr)
	{
		return VoiceService->ActivateVoiceInputWithRequestOptions(RequestOptions);
	}

	return false;
}


/**
 * Starts receiving voice input from the microphone and begins streaming it to for interpretation immediately
 *
 * @return true if the activation was successful
 */
bool AVoiceExperience::ActivateVoiceInputImmediately()
{
	if (VoiceService != nullptr)
	{
		return VoiceService->ActivateVoiceInputImmediately();
	}
	
	return false;
}

/**
 * Starts receiving voice input from the microphone and begins streaming it to VoiceService for interpretation
 * 
 * @param RequestOptions [in] the RequestOptions string, this could be used for Dynamic Entities
 *
 * @return true if the activation was successful
 */
bool AVoiceExperience::ActivateVoiceInputImmediatelyWithRequestOptions(const FString& RequestOptions)
{
	if (VoiceService != nullptr)
	{
		return VoiceService->ActivateVoiceInputImmediatelyWithRequestOptions(RequestOptions);
	}
	
	return false;
}

/**
 * Stops receiving voice input from the microphone and stops streaming it
 *
 * @return true if the deactivation was successful
 */
bool AVoiceExperience::DeactivateVoiceInput()
{
	if (VoiceService != nullptr)
	{
		return VoiceService->DeactivateVoiceInput();
	}
	
	return false;
}

/**
 * Stops receiving voice input from the microphone and stop receiving unreceived response.
 *
 * @return true if deactivation is successful otherwise false
 */
bool AVoiceExperience::DeactivateAndAbortRequest()
{
	if (VoiceService != nullptr)
	{
		return VoiceService->DeactivateAndAbortRequest();
	}
	
	return false;
}
/**
 * Is voice input active on this component?
 *
 * @return true if active otherwise false
 */
bool AVoiceExperience::IsVoiceInputActive() const
{
	if (VoiceService != nullptr)
	{
		return VoiceService->IsVoiceInputActive();
	}

	return false;
}

/**
 * Get the current voice input volume
 *
 * @return if voice capture is in progress returns the current volume otherwise 0
 */
float AVoiceExperience::GetVoiceInputVolume() const
{
	if (VoiceService != nullptr)
	{
		return VoiceService->GetVoiceInputVolume();
	}

	return -1.0f;
}

/**
 * Is voice streaming to active on this component?
 *
 * @return true if active otherwise false
 */
bool AVoiceExperience::IsVoiceStreamingActive() const
{
	if (VoiceService != nullptr)
	{
		return VoiceService->IsVoiceStreamingActive();
	}
	
	return false;
}

/**
 * Is any request currently in progress?
 *
 * @return true if in progress otherwise false
 */
bool AVoiceExperience::IsRequestInProgress() const
{
	if (VoiceService != nullptr)
	{
		return VoiceService->IsRequestInProgress();
	}
	
	return false;
}

/**
 * Sends a text string to for interpretation.
 *
 * @param Text [in] The string we want to interpret
 */
void AVoiceExperience::SendTranscription(const FString& Text)
{
	if (VoiceService != nullptr)
	{
		VoiceService->SendTranscription(Text);
	}
}

/**
 * Sends a text string to Wit for interpretation
 *
 * @param RequestOptions [in] the RequestOptions string, this could be used for Dynamic Entities
 * 
 * @param Text [in] the string we want to interpret
 */
void AVoiceExperience::SendTranscriptionWithRequestOptions(const FString& Text, const FString& RequestOptions)
{
	if (VoiceService != nullptr)
	{
		VoiceService->SendTranscriptionWithRequestOptions(Text, RequestOptions);
	}
}

/**
 * Accept the given Partial Response and cancel the current request
 *
 * @param Response [in] the Partial Response to accept, this will be used as final response to call onResponse.
 * 
 */
void AVoiceExperience::AcceptPartialResponseAndCancelRequest(const FWitResponse& Response)
{
	if (VoiceService != nullptr)
	{
		VoiceService->AcceptPartialResponseAndCancelRequest(Response);
	}
}
