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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer")
	FString SpeakerName{};

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
	virtual void SpeakPhrase(const FString& Phrase, const TSharedPtr<FJsonObject> ContextMap) override;
	virtual bool IsSpeaking(const TSharedPtr<FJsonObject> ContextMap) override;

	/**
	 * The key to use to lookup the speaker name in the context map
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer")
	FString SpeakerNameContextMapKey{TEXT("wit_composer_speaker")};

	/**
	 * The speakers that are available to be activated 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer")
	TArray<FComposerSpeaker> Speakers{};

private:

	/**
	 * Get a speaker give their name
	 */
	AWitTtsSpeaker* GetSpeaker(const TSharedPtr<FJsonObject> ContextMap) const;

	/**
	 * Lookup the speaker name from the context map
	 */
	FString GetSpeakerName(const TSharedPtr<FJsonObject> ContextMap) const;
};