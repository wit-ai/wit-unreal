/*
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
	 * The audio source used to play sounds
	 */
	UPROPERTY(VisibleAnywhere, Category="TTS")
	UAudioComponent* AudioComponent{};
	
	/**
	 * Speak a phrase with the default configuration
	 *
	 * @param TextToSpeak [in] text to speak
	 * @param QueueAudio [in] should audio be placed in a queue
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void Speak(const FString& TextToSpeak, const bool bQueueAudio = true);

	/**
	 * Speak a phrase with custom settings
	 *
	 * @param ClipSettings [in] the text and settings to speak
	 * @param QueueAudio [in] should audio be placed in a queue
	 */
	UFUNCTION(BlueprintCallable, Category="TTS")
	void SpeakWithSettings(const FTtsConfiguration& ClipSettings, bool bQueueAudio = true);

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

	/**
	* Queue of SoundWave files to be played
	*/
	TArray<USoundBase*> SoundWaveQueue{};

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

	/**
	 * Callback that is called when an audio playback is finished. Plays the next queued SoundWave, if present
	 */
	UFUNCTION()
	void OnAudioFinished();
	
};
