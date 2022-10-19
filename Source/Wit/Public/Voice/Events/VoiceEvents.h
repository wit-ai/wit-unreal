/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Wit/Request/WitResponse.h"
#include "Wit/Request/WitRequestConfiguration.h"
#include "VoiceEvents.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWitEventDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWitTranscriptionDelegate, const FString&, Transcription);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWitResponseDelegate, const bool, bIsSuccessful, const FWitResponse&, WitResponse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWitErrorDelegate, const FString&, ErrorMessage, const FString&, HumanReadableMessage);
DECLARE_DELEGATE_OneParam(FOnWitRequestCustomizeDelegate, FWitRequestConfiguration&);

/**
 * Container for all voice command events
 */
UCLASS(ClassGroup=(Meta))
class WIT_API UVoiceEvents final : public UActorComponent
{
	GENERATED_BODY()

public:
	
	/**
	 * The response we receive from Wit requests parsed into UObject structures. This response contains
	 * information about the meaning of the speech or text that was sent
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Voice")
	FWitResponse WitResponse{};

	/**
	 * Callback to call when a Wit request has been fully processed. The callback receives the full WitResponse
	 * which can be used to do any required processing
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitResponseDelegate OnWitResponse{};

	/**
	 * Callback to call whenever a partial Response is received from VoiceService
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitResponseDelegate OnWitPartialResponse{};

	/**
	 * Callback to call when there is a Wit error 
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitErrorDelegate OnWitError{};

	/**
	 * Callback to call whenever a partial transcription is received from VoiceService
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitTranscriptionDelegate OnPartialTranscription{};

	/**
	 * Callback to call when a full transcription is received from VoiceService
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitTranscriptionDelegate OnFullTranscription{};
	
	/**
	 * Called when voice capture starts capturing voice data
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitEventDelegate OnStartVoiceInput{};

	/**
	 * Called when voice capture stops capturing voice data
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitEventDelegate OnStopVoiceInput{};

	/**
	 * Called when voice capture stops due to the input volume dropping below the required threshold for too long
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitEventDelegate OnStopVoiceInputDueToInactivity{};

	/**
	 * Called when voice capture stops due to exceeding the maximum capture duration
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitEventDelegate OnStopVoiceInputDueToTimeout{};

	/**
	 * Called when voice capture stops due to a call to deactivate
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitEventDelegate OnStopVoiceInputDueToDeactivation{};

	/**
	 * Called when the minimum wake threshold is hit
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitEventDelegate OnMinimumWakeThresholdHit{};

	/**
	 * Called to give the opportunity to customize a voice request
	 * Note: this is deliberately not blueprint assignable because blueprint assignable delegates do not support non-const references 
	 */
	FOnWitRequestCustomizeDelegate OnRequestCustomize{};

};
