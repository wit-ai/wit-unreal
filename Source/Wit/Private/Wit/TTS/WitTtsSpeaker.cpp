/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/TTS/WitTtsSpeaker.h"
#include "Components/AudioComponent.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Wit speaker constructor
 */
AWitTtsSpeaker::AWitTtsSpeaker()
	: Super()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
}

/**
 * Called when play begins
 */
void AWitTtsSpeaker::BeginPlay()
{	
	if (EventHandler != nullptr)
	{
		UE_LOG(LogWit, Verbose, TEXT("BeginPlay: adding synthesize response callback"));
		
		EventHandler->OnSynthesizeResponse.AddUniqueDynamic(this, &AWitTtsSpeaker::OnSynthesizeResponse);
	}

	Super::BeginPlay();
}

/**
 * Called when the component is destroyed
 */
void AWitTtsSpeaker::BeginDestroy()
{
	Super::BeginDestroy();

	if (EventHandler != nullptr)
	{
		EventHandler->OnSynthesizeResponse.RemoveDynamic(this, &AWitTtsSpeaker::OnSynthesizeResponse);
	}
}

/**
 * Speak a phrase with the default configuration
 *
 * @param TextToSpeak [in] the text to speak
 */
void AWitTtsSpeaker::Speak(const FString& TextToSpeak)
{
	if (VoicePreset != nullptr)
	{
		FTtsConfiguration ClipSettings = VoicePreset->Synthesize;
		
		ClipSettings.Text = TextToSpeak;
		
		ConvertTextToSpeechWithSettings(ClipSettings);
	}
	else
	{
		ConvertTextToSpeech(TextToSpeak);
	}
}


/**
 * Speak a phrase with custom settings
 *
 * @param ClipSettings [in] the settings to use
 */
void AWitTtsSpeaker::SpeakWithSettings(const FTtsConfiguration& ClipSettings)
{
	ConvertTextToSpeechWithSettings(ClipSettings);
}

/**
 * Stop speaking
 */
void AWitTtsSpeaker::Stop()
{
	if (AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}
}

/**
 * Are we currently speaking?
 *
 * @return true if we are speaking
 */
bool AWitTtsSpeaker::IsSpeaking() const
{
	return AudioComponent->IsPlaying();
}

/**
 * Are we currently loading?
 *
 * @return true if we are loading
 */
bool AWitTtsSpeaker::IsLoading() const
{
	return IsRequestInProgress();
}

/**
 * Callback that is called when a Wit.ai response is received which is used to see if we have a match
 * 
 * @param bIsSuccessful [in] true if the response was successful
 * @param SoundWave [in] the generated sound wave
 */
void AWitTtsSpeaker::OnSynthesizeResponse(const bool bIsSuccessful, USoundWave* SoundWave)
{
	if (!bIsSuccessful)
	{
		return;
	}

	Stop();
	
	AudioComponent->SetSound(SoundWave);
	AudioComponent->Play();
}
