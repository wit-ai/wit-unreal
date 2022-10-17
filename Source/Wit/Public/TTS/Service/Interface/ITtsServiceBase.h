/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "TTS/Configuration/TtsConfiguration.h"

/**
 * This is the root interface for TTS services which just provides a pure C++ abstract interface
 */
class ITtsServiceBase
{
public:
	/** 
	 * Destructor for overrides 
	 */
	virtual ~ITtsServiceBase() = default;

	/**
	 * Is a Wit.ai request currently in progress?
	 *
	 * @return true if in progress otherwise false
	 */
	virtual bool IsRequestInProgress() const = 0;
	
	/**
	 * Sends a text string to Wit for conversion to speech with default settings
	 *
	 * @param TextToConvert [in] the string we want to convert to speech
	 */
	virtual void ConvertTextToSpeech(const FString& TextToConvert) = 0;

	/**
	 * Sends a text string to Wit for conversion to speech with custom settings
	 *
	 * @param ClipSettings [in] the string we want to convert to speech
	 */
	virtual void ConvertTextToSpeechWithSettings(const FTtsConfiguration& ClipSettings) = 0;

	/**
	 * Fetch a list of available voices from Wit
	 */
	virtual void FetchAvailableVoices() = 0;

};
