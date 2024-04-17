/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "WitTtsExperience.h"
#include "GameFramework/Actor.h"
#include "WitTtsSharedSpeaker.generated.h"

class UAudioComponent;

/**
 * Represents a speaker
 */
UCLASS( ClassGroup=(Meta) )
class WIT_API AWitTtsSharedSpeaker : public AActor
{
	GENERATED_BODY()
	
public:

	/**
	 * Sets default values for this actor's properties
	 */
	AWitTtsSharedSpeaker();

	/**
	 * The tag of the TTS experience to use in case there is more than one. If the tag is empty use the first found
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TTS")
	FName TtsExperienceTag{};

	/**
	 * The audio source used to play sounds
	 */
	UPROPERTY(VisibleAnywhere, Category="TTS")
	UAudioComponent* AudioComponent{};
	
	/**
	 * Speak a phrase with the default configuration
	 *
	 * @param TextToSpeak [in] the text to speak
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void Speak(const FString& TextToSpeak);

	/**
	 * Speak a phrase with custom settings
	 *
	 * @param ClipSettings [in] the settings to use
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void SpeakWithSettings(const FTtsConfiguration& ClipSettings);

	/**
	 * Stop speaking
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void Stop();
	
	/**
	 * Are we currently speaking?
	 *
	 * @return true if we are speaking
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	bool IsSpeaking() const;

	/**
	 * Are we currently loading?
	 *
	 * @return true if we are loading
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	bool IsLoading() const;
	
protected:

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	
	/**
	 * Callback that is called when a Wit.ai response is received which is used to see if we have a match
	 * 
	 * @param bIsSuccessful [in] true if the response was successful
	 * @param SoundBase [in] the generated sound wave
	 */
	UFUNCTION()
	void OnSynthesizeResponse(const bool bIsSuccessful, USoundBase* SoundBase);

private:
	
	/**
	 * The TTS experience that the speaker will use
	 */
	UPROPERTY(Transient)
	ATtsExperience* TtsExperience{};
	
};
