﻿/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "WitTtsExperience.h"
#include "WitTtsSpeaker.generated.h"

class UAudioComponent;

/**
 * Represents a speaker
 */
UCLASS( ClassGroup=(Meta) )
class WIT_API AWitTtsSpeaker : public AWitTtsExperience
{
	GENERATED_BODY()
	
public:

	/**
	 * Sets default values for this actor's properties
	 */
	AWitTtsSpeaker();
	
	/**
	 * The audio source to play sounds
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
	 * The memory cache to use
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void SpeakWithSettings(const FTtsConfiguration& ClipSettings);

	/**
	 * The storage cache to use
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void Stop();
	
	/**
	 * Speak a phrase with the default configuration
	 *
	 * @return true if speaking
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	bool IsSpeaking() const;

	/**
	 * Speak a phrase with custom settings
	 *
	 * @return true if speech is loading
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
	 * @param SoundWave [in] the generated sound wave
	 */
	UFUNCTION()
	void OnSynthesizeResponse(const bool bIsSuccessful, USoundWave* SoundWave);
	
};
