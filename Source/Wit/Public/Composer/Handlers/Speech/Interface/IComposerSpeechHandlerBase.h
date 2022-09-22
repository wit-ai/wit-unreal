/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Composer/Data/ComposerContextMap.h"

/**
 * Interface for implementing actions in a composer graph
 */
class IComposerSpeechHandlerBase
{
public:

	/** 
	 * Destructor for overrides 
	 */
	virtual ~IComposerSpeechHandlerBase() = default;

	/**
	 * Speaks the specified phrase
	 *
	 * @param Phrase [in] the text to speak
	 * @param ContextMap [in] the context map
	 */
	virtual void SpeakPhrase(const FString& Phrase, const UComposerContextMap* ContextMap) = 0;

	/**
	 * Checks to see if we are in the middle of speaking
	 *
	 * @param ContextMap [in] the context map
	 *
	 * @return true if we are currently speaking
	 */
	virtual bool IsSpeaking(const UComposerContextMap* ContextMap) const = 0;
	
};
