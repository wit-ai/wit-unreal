/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "WitTtsExperience.h"
#include "GameFramework/Actor.h"
#include "WitTtsSpeaker.generated.h"

class UAudioComponent;

/**
 * Represents a speaker
 */
UCLASS( ClassGroup=(Meta) )
class WIT_API AWitTtsSpeaker : public AActor
{
	GENERATED_BODY()
	
public:

	/**
	 * Sets default values for this actor's properties
	 */
	AWitTtsSpeaker();
	
	/**
	 * The Wit TTS Voice Preset that the speaker will use
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker|Configuration")
	UTtsVoicePresetAsset* VoicePreset{};

	/**
	 * The tag of the TTS experience to use in case there is more than one. If the tag is empty use the first found
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Speaker|Configuration")
	FName ExperienceTag{};

	/**
	 * The audio source to play sounds
	 */
	UPROPERTY(VisibleAnywhere, Category="Speaker|Audio")
	UAudioComponent* AudioComponent{};
	
	/**
	 * Speak a phrase with the default configuration
	 *
	 * @param TextToSpeak [in] the text to speak
	 */
	UFUNCTION(BlueprintCallable, Category="Speaker")
	void Speak(const FString& TextToSpeak) const;

	/**
	 * The memory cache to use
	 */
	UFUNCTION(BlueprintCallable, Category="Speaker")
	void SpeakWithSettings(const FTtsConfiguration& ClipSettings) const;

	/**
	 * The storage cache to use
	 */
	UFUNCTION(BlueprintCallable, Category="Speaker")
	void Stop();
	
	/**
	 * Speak a phrase with the default configuration
	 *
	 * @param TextToSpeak [in] the text to speak
	 */
	UFUNCTION(BlueprintCallable, Category="Speaker")
	bool IsSpeaking() const;

	/**
	 * Speak a phrase with custom settings
	 *
	 * @param ClipSettings [in] the settings to use
	 */
	UFUNCTION(BlueprintCallable, Category="Speaker")
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

private:
	
	/**
	 * The TTS experience that the speaker will use
	 */
	UPROPERTY(Transient)
	ATtsExperience* TtsExperience{};
	
};
