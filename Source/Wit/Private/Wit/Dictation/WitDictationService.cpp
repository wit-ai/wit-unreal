/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Dictation/WitDictationService.h"

#include "Wit/Request/WitRequestBuilder.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Called when the component starts playing
 */
UWitDictationService::UWitDictationService()
	: Super()
{
}

/**
 * Called when the component starts playing
 */
void UWitDictationService::BeginPlay()
{
	VoiceExperience = FWitHelperUtilities::FindVoiceExperience(GetWorld(), Configuration != nullptr ? Configuration->VoiceExperienceTag : FName());
	
	if (VoiceExperience != nullptr && VoiceExperience->VoiceEvents != nullptr)
	{
		UE_LOG(LogWit, Verbose, TEXT("BeginPlay: adding request customise callback"));
		
		VoiceExperience->VoiceEvents->OnRequestCustomize.BindUObject(this, &UWitDictationService::OnDictationRequestCustomize);
		VoiceExperience->VoiceEvents->OnFullTranscription.AddUniqueDynamic(this, &UWitDictationService::OnFullTranscription);
		VoiceExperience->VoiceEvents->OnPartialTranscription.AddUniqueDynamic(this, &UWitDictationService::OnPartialTranscription);
		VoiceExperience->VoiceEvents->OnStartVoiceInput.AddUniqueDynamic(this, &UWitDictationService::OnStartVoiceInput);
		VoiceExperience->VoiceEvents->OnStopVoiceInput.AddUniqueDynamic(this, &UWitDictationService::OnStopVoiceInput);
		VoiceExperience->VoiceEvents->OnWitResponse.AddUniqueDynamic(this, &UWitDictationService::OnWitResponse);
		VoiceExperience->VoiceEvents->OnWitError.AddUniqueDynamic(this, &UWitDictationService::OnWitError);
	}

	Super::BeginPlay();
}

/**
 * Called when the component is destroyed
 */
void UWitDictationService::BeginDestroy()
{
	if (VoiceExperience != nullptr && VoiceExperience->VoiceEvents != nullptr)
	{
		UE_LOG(LogWit, Verbose, TEXT("BeginDestroy: removing request customise callback"));
		
		VoiceExperience->VoiceEvents->OnRequestCustomize.Unbind();
		VoiceExperience->VoiceEvents->OnFullTranscription.RemoveDynamic(this, &UWitDictationService::OnFullTranscription);
		VoiceExperience->VoiceEvents->OnPartialTranscription.RemoveDynamic(this, &UWitDictationService::OnPartialTranscription);
		VoiceExperience->VoiceEvents->OnStartVoiceInput.RemoveDynamic(this, &UWitDictationService::OnStartVoiceInput);
		VoiceExperience->VoiceEvents->OnStopVoiceInput.RemoveDynamic(this, &UWitDictationService::OnStopVoiceInput);
		VoiceExperience->VoiceEvents->OnWitResponse.RemoveDynamic(this, &UWitDictationService::OnWitResponse);
		VoiceExperience->VoiceEvents->OnWitError.RemoveDynamic(this, &UWitDictationService::OnWitError);
	}
	
	Super::BeginDestroy();
}

/**
 * Starts receiving dictation from the microphone and begins streaming it to Wit.ai for interpretation
 *
 * @return true if the activation was successful
 */
bool UWitDictationService::ActivateDictation()
{
	if (VoiceExperience != nullptr)
	{
		return VoiceExperience->ActivateVoiceInput();	
	}
	
	return false;
}

/**
 * Starts receiving dictation from the microphone and begins streaming it to Wit.ai for interpretation
 *
 * @return true if the activation was successful
 */
bool UWitDictationService::ActivateDictationWithRequestOptions(const FString& RequestOptions) 
{
	if (VoiceExperience != nullptr)
	{
		return VoiceExperience->ActivateVoiceInputWithRequestOptions(RequestOptions);	
	}
	
	return false;
}

/**
 * Starts receiving dictation from the microphone and begins streaming it to Wit.ai for interpretation
 *
 * @return true if the activation was successful
 */
bool UWitDictationService::ActivateDictationImmediately()
{
	if (VoiceExperience != nullptr)
	{
		return VoiceExperience->ActivateVoiceInputImmediately();
	}
	
	return false;
}

/**
 * Stops receiving dictation from the microphone and stops streaming it to Wit.ai
 *
 * @return true if the deactivation was successful
 */
bool UWitDictationService::DeactivateDictation()
{
	if (VoiceExperience != nullptr)
	{
		return VoiceExperience->DeactivateVoiceInput();	
	}
	
	return false;
}

/**
 * Is dictation active on this component?
 *
 * @return true if active otherwise false
 */
bool UWitDictationService::IsDictationActive() const
{
	if (VoiceExperience != nullptr)
	{
		return VoiceExperience->IsVoiceInputActive();	
	}
	
	return false;
}

/**
 * Is any Wit.ai request currently in progress?
 *
 * @return true if in progress otherwise false
 */
bool UWitDictationService::IsRequestInProgress() const
{
	if (VoiceExperience != nullptr)
	{
		return VoiceExperience->IsRequestInProgress();	
	}
	
	return false;
}

/**
 * Callback to redirect the speech endpoint to the dictation equivalent
 */
void UWitDictationService::OnDictationRequestCustomize(FWitRequestConfiguration& RequestConfiguration)
{	
	// Redirect the endpoint from the normal voice endpoint to the corresponding dictation endpoint

	const FString SpeechEndpoint = FWitRequestBuilder::GetEndpointString(EWitRequestEndpoint::Speech);

	const bool bIsSpeechEndpointRedirected = RequestConfiguration.Endpoint.Equals(SpeechEndpoint, ESearchCase::IgnoreCase);

	if (bIsSpeechEndpointRedirected)
	{
		UE_LOG(LogWit, Verbose, TEXT("OnDictationRequestCustomize: redirecting speech endpoint"));
		
		RequestConfiguration.Endpoint = FWitRequestBuilder::GetEndpointString(EWitRequestEndpoint::Dictation);
	}
}

/**
 * Callback to catch and pass on the full transcription event
 */
void UWitDictationService::OnFullTranscription(const FString& Transcription)
{
	if (Events != nullptr)
	{
		Events->OnFullTranscription.Broadcast(Transcription);
	}
}

/**
 * Callback to catch and pass on the partial transcription event
 */
void UWitDictationService::OnPartialTranscription(const FString& Transcription)
{
	if (Events != nullptr)
	{
		Events->OnPartialTranscription.Broadcast(Transcription);
	}
}

/**
 * Callback to catch and pass on the start voice input event
 */
void UWitDictationService::OnStartVoiceInput()
{
	if (Events != nullptr)
	{
		Events->OnStartDictation.Broadcast();
	}
}

/**
 * Callback to catch and pass on the stop voice input event
 */
void UWitDictationService::OnStopVoiceInput()
{
	if (Events != nullptr)
	{
		Events->OnStopDictation.Broadcast();
	}
}

/**
 * Callback to catch and pass on the wit response event
 */
void UWitDictationService::OnWitResponse(const bool bIsSuccessful, const FWitResponse& WitResponse)
{
	if (Events != nullptr)
	{
		Events->OnWitResponse.Broadcast(bIsSuccessful, WitResponse);
	}
}

/**
 * Callback to catch and pass on the wit error event
 */
void UWitDictationService::OnWitError(const FString& ErrorMessage, const FString& HumanReadableMessage)
{
	if (Events != nullptr)
	{
		Events->OnWitError.Broadcast(ErrorMessage, HumanReadableMessage);
	}
}
