/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Voice/Events/VoiceEvents.h"
#include "DictationEvents.generated.h"

/**
 * Container for all Dictation events
 */
UCLASS(ClassGroup=(Meta))
class WIT_API UDictationEvents final : public UActorComponent
{
	GENERATED_BODY()

public:
	
	/**
	 * Callback to call when a Wit request has been fully processed. The callback receives the full WitResponse
	 * which can be used to do any required processing
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitResponseDelegate OnWitResponse{};
	
	/**
	 * Callback to call when there is a Wit error 
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitErrorDelegate OnWitError{};

	/**
	 * Callback to call whenever a partial transcription is received 
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitTranscriptionDelegate OnPartialTranscription{};

	/**
	 * Callback to call when a full transcription is received 
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitTranscriptionDelegate OnFullTranscription{};

	/**
	 * Called when dictation starts
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitEventDelegate OnStartDictation{};

	/**
	 * Called when dictations stops for any reason
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitEventDelegate OnStopDictation{};

	// TODO: dictation session start/end events

};
