/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "ComposerSpeechHandler.h"
#include "Wit/TTS/WitTtsSpeaker.h"
#include "ComposerSpeechDefaultHandler.generated.h"

/**
 * Provides a mapping between speaker name and speaker
 */
USTRUCT(BlueprintType)
struct WIT_API FComposerSpeaker
{
	GENERATED_BODY()

	/** The speaker's name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer")
	FString SpeakerName{};

	/** The associated speaker actor to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer")
	AWitTtsSpeaker* Speaker{};
	
};

/**
 * Default class for implementing a composer action handler. 
 */
UCLASS(ClassGroup=(Meta), meta=(BlueprintSpawnableComponent))
class WIT_API UComposerSpeechDefaultHandler : public UComposerSpeechHandler
{
	GENERATED_BODY()

public:
	
	/**
	 * IComposerSpeechHandlerBase default implementation
	 */
	virtual void SpeakPhrase(const FString& Phrase, const UComposerContextMap* ContextMap) override;
	virtual bool IsSpeaking(const UComposerContextMap* ContextMap) const override;

	/**
	 * The key to use to lookup the speaker name in the context map
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer|Speaker")
	FString SpeakerNameContextMapKey{TEXT("wit_composer_speaker")};

	/**
	 * The speakers that are available to be activated 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer|Speaker")
	TArray<FComposerSpeaker> Speakers{};

private:

	/**
	 * Get a speaker give their name
	 */
	AWitTtsSpeaker* GetSpeaker(const UComposerContextMap* ContextMap) const;

	/**
	 * Lookup the speaker name from the context map
	 */
	FString GetSpeakerName(const UComposerContextMap* ContextMap) const;
};