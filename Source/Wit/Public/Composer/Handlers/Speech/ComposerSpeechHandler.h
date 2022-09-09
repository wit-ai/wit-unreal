/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/IComposerSpeechHandlerBase.h"
#include "ComposerSpeechHandler.generated.h"

/**
 * Abstract class for implementing a composer action handler. This should be the base if you want to implement your own action handler implementation
 */
UCLASS(NotBlueprintable, Abstract)
class WIT_API UComposerSpeechHandler : public UActorComponent, public IComposerSpeechHandlerBase
{
	GENERATED_BODY()

public:
	
	/**
	 * IComposerSpeechHandlerBase default implementation
	 */
	virtual void SpeakPhrase(const FString& Phrase, const UComposerContextMap* ContextMap) override {};
	virtual bool IsSpeaking(const UComposerContextMap* ContextMap) const override { return false; };
	
};
