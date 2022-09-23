/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Composer/WitComposerService.h"
#include "JsonObjectConverter.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Wit/Request/WitRequestBuilder.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Constructor
 */
UWitComposerService::UWitComposerService()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = true;
}

/**
 * Called when play begins
 */
void UWitComposerService::BeginPlay()
{
	VoiceExperience = FWitHelperUtilities::FindVoiceExperience(GetWorld(), Configuration != nullptr ? Configuration->VoiceExperienceTag : FName());
	
	if (VoiceExperience != nullptr && VoiceExperience->VoiceEvents != nullptr)
	{
		UE_LOG(LogWit, Verbose, TEXT("BeginPlay: adding request customise callback"));
		
		VoiceExperience->VoiceEvents->OnRequestCustomize.BindUObject(this, &UWitComposerService::OnComposerRequestCustomize);
	}

	Super::BeginPlay();
}

/**
 * Called when about to be destroyed
 */
void UWitComposerService::BeginDestroy()
{
	if (VoiceExperience != nullptr && VoiceExperience->VoiceEvents != nullptr)
	{
		UE_LOG(LogWit, Verbose, TEXT("BeginDestroy: removing request customise callback"));
		
		VoiceExperience->VoiceEvents->OnRequestCustomize.Unbind();
	}
	
	Super::BeginDestroy();
}

/**
 * Updates the component every frame
 *
 * @param DeltaTime [in] the time in seconds that has pass since the last frame
 * @param TickType [in] the kind of tick this is, for example, are we paused, or 'simulating' in the editor
 * @param ThisTickFunction [in] internal tick function struct that caused this to run
 */
void UWitComposerService::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsWaitingToContinue)
	{
		return;
	}

	// Wait for a period of time after continuing is allowed
	
	const bool bIsWaitingForDelay = ContinueDelayTimer > 0.0f;
	
	if (bIsWaitingForDelay)
	{
		ContinueDelayTimer -= DeltaTime;

		const bool bIsDelayFinished = ContinueDelayTimer <= 0.0f;
		
		if (bIsDelayFinished)
		{
			DoContinue();
		}

		return;
	}

	// Check to see if we are ok to proceed
	
	if (CanContinue())
	{
		const bool bIsDelayRequired = Configuration != nullptr && Configuration->ContinueDelay > 0.0f;

		if (bIsDelayRequired)
		{
			ContinueDelayTimer = Configuration->ContinueDelay;
		}
		else
		{
			DoContinue();
		}
	}
}

/**
 * Set the handlers to use
 */
void UWitComposerService::SetHandlers(UComposerEvents* EventHandlerToUse, UComposerActionHandler* ActionHandlerToUse, UComposerSpeechHandler* SpeechHandlerToUse)
{
	EventHandler = EventHandlerToUse;
	ActionHandler = ActionHandlerToUse;
	SpeechHandler = SpeechHandlerToUse;
}

/**
 * Set the configuration to use
 */
void UWitComposerService::SetConfiguration(const FComposerConfiguration* ConfigurationToUse)
{
	Configuration = ConfigurationToUse;
}

/**
 * Start a new composer session
 */
void UWitComposerService::StartSession(FString NewSessionId)
{
	if (NewSessionId.IsEmpty())
	{
		NewSessionId = GetDefaultSessionId();
	}

	UE_LOG(LogWit, Verbose, TEXT("StartSession: starting session with id %s"), *NewSessionId);
	
	SessionId = NewSessionId;
	SessionStart = FDateTime::UtcNow();

	if (EventHandler != nullptr)
	{
		EventHandler->OnComposerSessionBegin.Broadcast(CurrentContextMap);
	}
}

/**
 * End the current composer session
 */
void UWitComposerService::EndSession()
{
	if (SessionId.IsEmpty())
	{
		return;
	}

	SessionId.Empty();

	if (EventHandler != nullptr)
	{
		EventHandler->OnComposerSessionEnd.Broadcast(CurrentContextMap);
	}
}

/**
 * Get a default session id
 */
FString UWitComposerService::GetDefaultSessionId()
{
	const FTimespan TimeSinceEpoch = FDateTime::UtcNow() - FDateTime(1970,1,1);

	const FString Timestamp = TimeSinceEpoch.ToString();
	const FString Guid = FGuid::NewGuid().ToString();

	return Guid + TEXT("-") + Timestamp;
}

/**
 * Update the context map
 */
void UWitComposerService::SetContextMap(UComposerContextMap* NewContextMap)
{
	if (CurrentContextMap == NewContextMap)
	{
		return;
	}

	CurrentContextMap = NewContextMap;

	if (EventHandler != nullptr)
	{
		EventHandler->OnComposerContextMapChange.Broadcast(CurrentContextMap);
	}
}

/**
 * Callback to redirect the speech and message endpoints to the composer equivalents
 */
void UWitComposerService::OnComposerRequestCustomize(FWitRequestConfiguration& RequestConfiguration)
{
	if (Configuration == nullptr || !Configuration->bShouldRouteVoiceServiceToComposer)
	{
		return;
	}

	if (SessionId.IsEmpty())
	{
		StartSession(FString());
	}

	if (CurrentContextMap == nullptr)
	{
		SetContextMapInternal(MakeShared<FJsonObject>());
	}

	if (EventHandler != nullptr)
	{
		EventHandler->OnComposerActivation.Broadcast(CurrentContextMap);	
	}
	
	// Redirect the endpoints from the normal voice endpoints to the corresponding composer endpoints. We also need to add
	// on additional parameters that composer expects

	const FString MessageEndpoint = FWitRequestBuilder::GetEndpointString(EWitRequestEndpoint::Message);
	const FString SpeechEndpoint = FWitRequestBuilder::GetEndpointString(EWitRequestEndpoint::Speech);

	const bool bIsMessageEndpointRedirected = RequestConfiguration.Endpoint.Equals(MessageEndpoint, ESearchCase::IgnoreCase);
	const bool bIsSpeechEndpointRedirected = RequestConfiguration.Endpoint.Equals(SpeechEndpoint, ESearchCase::IgnoreCase);
	
	if (bIsMessageEndpointRedirected)
	{
		UE_LOG(LogWit, Verbose, TEXT("OnComposerRequestCustomize: redirecting message endpoint"));

		RequestConfiguration.Endpoint = FWitRequestBuilder::GetEndpointString(EWitRequestEndpoint::Event);
	}
	else if (bIsSpeechEndpointRedirected)
	{
		UE_LOG(LogWit, Verbose, TEXT("OnComposerRequestCustomize: redirecting speech endpoint"));
		
		RequestConfiguration.Endpoint = FWitRequestBuilder::GetEndpointString(EWitRequestEndpoint::Converse);
	}

	const bool bIsEndpointRedirected = bIsMessageEndpointRedirected || bIsSpeechEndpointRedirected;

	if (!bIsEndpointRedirected)
	{
		return;
	}

	// For composer there's a couple of extra parameters we need to add on to the request
	
	FWitRequestBuilder::AddParameter(RequestConfiguration, EWitParameter::SessionId, FGenericPlatformHttp::UrlEncode(SessionId));

	const bool bIsValidContextMap = CurrentContextMap != nullptr && CurrentContextMap->GetJsonObject() != nullptr;
	
	if (bIsValidContextMap)
	{
		FString ContextJsonString;
	
		const TSharedRef<TJsonWriter<TCHAR>> Writer = TJsonWriterFactory<TCHAR>::Create(&ContextJsonString);
		FJsonSerializer::Serialize(CurrentContextMap->GetJsonObject().ToSharedRef(), Writer);

		FWitRequestBuilder::AddParameter(RequestConfiguration, EWitParameter::ContextMap, FGenericPlatformHttp::UrlEncode(ContextJsonString));
	}
		
	// Listen in to the raw response as it's different than a normal speech/message response and we will need to parse it to a different UStruct
	
	RequestConfiguration.OnRequestError.AddUObject(this, &UWitComposerService::OnComposerError);
	RequestConfiguration.OnRequestComplete.AddUObject(this, &UWitComposerService::OnComposerResponse);
}

/**
 * Callback when we receive a wit response
 */
void UWitComposerService::OnComposerResponse(const TArray<uint8>& BinaryResponse, TSharedPtr<FJsonObject> JsonResponse)
{
	const bool bIsConversionError = !FJsonObjectConverter::JsonObjectToUStruct(JsonResponse.ToSharedRef(), &ComposerResponse);
	
	if (bIsConversionError)
	{
		OnComposerError(TEXT("Json To UStruct failed"), TEXT("Convering the Json response to a UStruct failed"));
		return;
	}

	// Try to update the session context from the one returned in the response
	
	const TSharedPtr<FJsonObject>* ResponseContextMap = nullptr;
	JsonResponse->TryGetObjectField(TEXT("context_map"), ResponseContextMap);

	if (ResponseContextMap != nullptr)
	{
		SetContextMapInternal(*ResponseContextMap);
	}
	
	UE_LOG(LogWit, Verbose, TEXT("UStruct - Expects input (%d) action (%s) text (%s)"), ComposerResponse.Expects_Input, *ComposerResponse.Action, *ComposerResponse.Response.Text);

	if (EventHandler != nullptr)
	{
		EventHandler->OnComposerResponse.Broadcast(CurrentContextMap);
	}

	bool bShouldContinue = false;

	// If we receive a text phrase then we attempt to speak it
	
	const bool bShouldSpeak = !ComposerResponse.Response.Text.IsEmpty();
	
	if (bShouldSpeak)
	{
		bShouldContinue = true;

		DoSpeakPhrase(ComposerResponse.Response.Text);
	}

	// If we receive an action then we attempt to perform it
	
	const bool bShouldPerformAction = !ComposerResponse.Action.IsEmpty();

	if (bShouldPerformAction)
	{
		bShouldContinue = true;

		DoPerformAction(ComposerResponse.Action);
	}

	if (ComposerResponse.Expects_Input)
	{
		bShouldContinue = true;
	}

	if (bShouldContinue)
	{
		UE_LOG(LogWit, Verbose, TEXT("OnComposerResponse - waiting to continue"));

		bIsWaitingToContinue = true;
	}
}

/**
 * Internal function for setting the context map from a json object
 */
void UWitComposerService::SetContextMapInternal(TSharedPtr<FJsonObject> ContextMapJsonObject)
{
	if (CurrentContextMap == nullptr)
	{
		CurrentContextMap = NewObject<UComposerContextMap>();
	}

	if (CurrentContextMap->GetJsonObject() != ContextMapJsonObject)
	{
		CurrentContextMap->SetJsonObject(ContextMapJsonObject);

		if (EventHandler != nullptr)
		{
			EventHandler->OnComposerContextMapChange.Broadcast(CurrentContextMap);
		}
	}
}

/**
 * Callback when we receive a wit error
 */
void UWitComposerService::OnComposerError(const FString& ErrorMessage, const FString& HumanReadableMessage)
{
	ComposerResponse.Expects_Input = false;
	ComposerResponse.Action.Empty();
	ComposerResponse.Response.Reset();

	if (EventHandler != nullptr)
	{
		EventHandler->OnComposerError.Broadcast(CurrentContextMap);
	}
}

/**
 * Speak a given phrase
 */
void UWitComposerService::DoSpeakPhrase(const FString& Phrase) const
{
	UE_LOG(LogWit, Verbose, TEXT("DoSpeakPhrase - trying to speak phrase (%s)"), *Phrase);

	if (EventHandler != nullptr)
	{
		EventHandler->OnComposerSpeakPhrase.Broadcast(CurrentContextMap);
	}

	if (SpeechHandler != nullptr)
	{
		SpeechHandler->SpeakPhrase(Phrase, CurrentContextMap);
	}
}

/**
 * Perform a given action
 */
void UWitComposerService::DoPerformAction(const FString& Action) const
{
	UE_LOG(LogWit, Verbose, TEXT("DoPerformAction - trying to perform action (%s)"), *Action);

	if (EventHandler != nullptr)
	{
		EventHandler->OnComposerPerformAction.Broadcast(CurrentContextMap);
	}

	if (ActionHandler != nullptr)
	{
		ActionHandler->PerformAction(Action, CurrentContextMap);
	}
}

/**
 * Continue on after an action/speech
 */
void UWitComposerService::DoContinue()
{
	UE_LOG(LogWit, Verbose, TEXT("DoContinue - trying to continue"));

	bIsWaitingToContinue = false;
	
	if (ComposerResponse.Expects_Input)
	{
		UE_LOG(LogWit, Verbose, TEXT("DoContinue - activating input"));

		if (EventHandler != nullptr)
		{
			EventHandler->OnComposerExpectsInput.Broadcast(CurrentContextMap);
		}
		
		const bool bShouldAutoActivateInput = VoiceExperience != nullptr && Configuration != nullptr && Configuration->bShouldAutoActivateInput;

		if (bShouldAutoActivateInput)
		{
			VoiceExperience->ActivateVoiceInput();
		}
	}
	else
	{
		UE_LOG(LogWit, Verbose, TEXT("DoContinue - graph is complete - nothing more to do"));
		
		if (EventHandler != nullptr)
		{
			EventHandler->OnComposerComplete.Broadcast(CurrentContextMap);
		}

		const bool bShouldEndSession = Configuration != nullptr && Configuration->bShouldAutoEndSession;

		if (bShouldEndSession)
		{
			EndSession();
		}

		const bool bShouldClearContextMap = Configuration != nullptr && Configuration->bShouldAutoClearContextMap;

		if (bShouldClearContextMap)
		{
			SetContextMapInternal(MakeShared<FJsonObject>());
		}
	}
}

/**
 * Can we continue after an action/speech?
 */
bool UWitComposerService::CanContinue() const
{
	const bool bIsVoiceServiceActive = VoiceExperience != nullptr && VoiceExperience->IsRequestInProgress();

	if (bIsVoiceServiceActive)
	{
		UE_LOG(LogWit, VeryVerbose, TEXT("CanContinue: voice service is active exiting"));
		return false;
	}

	const bool bIsSpeechActive = SpeechHandler != nullptr && SpeechHandler->IsSpeaking(CurrentContextMap);

	if (bIsSpeechActive)
	{
		UE_LOG(LogWit, VeryVerbose, TEXT("CanContinue: speech is active exiting"));
		return false;
	}

	const bool bIsActionActive = ActionHandler != nullptr && ActionHandler->IsPerformingAction(ComposerResponse.Action);

	if (bIsActionActive)
	{
		UE_LOG(LogWit, VeryVerbose, TEXT("CanContinue: action is active exiting"));
		return false;
	}

	UE_LOG(LogWit, VeryVerbose, TEXT("CanContinue: ready to continue"));

	return true;
}
