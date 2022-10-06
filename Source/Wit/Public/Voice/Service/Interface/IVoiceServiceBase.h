/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Wit/Request/WitResponse.h"

/**
 * This is the root interface for voice services which just provides a pure C++ abstract interface
 */
class IVoiceServiceBase
{
public:
	/** 
	 * Destructor for overrides 
	 */
	virtual ~IVoiceServiceBase() = default;

	/**
	 * Starts receiving voice input from the microphone and begins streaming it to VoiceService for interpretation
	 *
	 * @return true if the activation was successful
	 */
	virtual bool ActivateVoiceInput() = 0;
	
	/**
	 * Starts receiving voice input from the microphone and begins streaming it to VoiceService for interpretation
	 *
	 * @param RequestOptions [in] the RequestOptions string, this could be used for Dynamic Entities
	 * 
	 * @return true if the activation was successful
	 */
	virtual bool ActivateVoiceInputWithRequestOptions(const FString& RequestOptions) = 0;

	/**
	 * Starts receiving voice input from the microphone and begins streaming it to VoiceService for interpretation
	 *
	 * @return true if the activation was successful
	 */
	virtual bool ActivateVoiceInputImmediately() = 0;
	
	/**
	 * Starts receiving voice input from the microphone and begins streaming it to VoiceService for interpretation
	 * 
	 * @param RequestOptions [in] the RequestOptions string, this could be used for Dynamic Entities
	 *
	 * @return true if the activation was successful
	 */
	virtual bool ActivateVoiceInputImmediatelyWithRequestOptions(const FString& RequestOptions) = 0;

	/**
	 * Stops receiving voice input from the microphone and stops streaming it to VoiceService
	 *
	 * @return true if deactivation is successful otherwise false
	 */
	virtual bool DeactivateVoiceInput() = 0;
	
	/**
	 * Stops receiving voice input from the microphone and stop receiving unreceived response.
	 *
	 * @return true if deactivation is successful otherwise false
	 */
	virtual bool DeactivateAndAbortRequest() = 0;

	/**
	 * Is the voice input currently active on this component?
	 *
	 * @return true if active otherwise false
	 */
	virtual bool IsVoiceInputActive() const = 0;

	/**
	 * Get the current voice input volume
	 *
	 * @return the current volume
	 */
	virtual float GetVoiceInputVolume() const = 0;

	/**
	 * Is the voice streaming to VoiceService currently active on this component?
	 *
	 * @return true if active otherwise false
	 */
	virtual bool IsVoiceStreamingActive() const = 0;

	/**
	 * Is a VoiceService request currently in progress?
	 *
	 * @return true if in progress otherwise false
	 */
	virtual bool IsRequestInProgress() const = 0;

	/**
	 * Sends a text string to Wit for interpretation
	 *
	 * @param Text [in] the string we want to interpret
	 */
	virtual void SendTranscription(const FString& Text) = 0;
	
	/**
	 * Sends a text string to Wit for interpretation
	 *
	 * @param RequestOptions [in] the RequestOptions string, this could be used for Dynamic Entities
	 * @param Text [in] the string we want to interpret
	 */
	virtual void SendTranscriptionWithRequestOptions(const FString& Text, const FString& RequestOptions) = 0;

	/**
	 * Accept the given Partial Response and cancel the current request
	 *
	 * @param Response [in] the Partial Response to accept, this will be used as final response to call onResponse.
	 */
	virtual void AcceptPartialResponseAndCancelRequest(const FWitResponse& Response) = 0;

};
