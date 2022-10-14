/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"

/**
 * This is the root interface for Dictation services which just provides a pure C++ abstract interface
 */
class IDictationServiceBase
{
public:

	/** 
	 * Destructor for overrides 
	 */
	virtual ~IDictationServiceBase() = default;

	/**
	 * Starts receiving dictation from the microphone and begins streaming it to VoiceService for interpretation
	 *
	 * @return true if the activation was successful
	 */
	virtual bool ActivateDictation() = 0;
	
	/**
	 * Starts receiving dictation from the microphone and begins streaming it to VoiceService for interpretation
	 *
	 * @param RequestOptions [in] the RequestOptions string, this could be used for Dynamic Entities
	 * 
	 * @return true if the activation was successful
	 */
	virtual bool ActivateDictationWithRequestOptions(const FString& RequestOptions) = 0;

	/**
	 * Starts receiving dictation from the microphone and begins streaming it to VoiceService for interpretation
	 *
	 * @return true if the activation was successful
	 */
	virtual bool ActivateDictationImmediately() = 0;
	
	/**
	 * Stops receiving dictation from the microphone and stops streaming it to VoiceService
	 *
	 * @return true if deactivation is successful otherwise false
	 */
	virtual bool DeactivateDictation() = 0;
	
	/**
	 * Is the dictation currently active on this component?
	 *
	 * @return true if active otherwise false
	 */
	virtual bool IsDictationActive() const = 0;
	
	/**
	 * Is a dictation service request currently in progress?
	 *
	 * @return true if in progress otherwise false
	 */
	virtual bool IsRequestInProgress() const = 0;
	
};
