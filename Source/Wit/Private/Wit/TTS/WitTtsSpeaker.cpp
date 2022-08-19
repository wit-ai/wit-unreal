/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/TTS/WitTtsSpeaker.h"

#include "Components/AudioComponent.h"
#include "TTS/Cache/Memory/TtsMemoryCache.h"
#include "TTS/Cache/Storage/TtsStorageCache.h"

/**
 * Wit speaker constructor
 */
AWitTtsSpeaker::AWitTtsSpeaker()
	: Super()
{
	PrimaryActorTick.bCanEverTick = false;

	WitTtsService = CreateDefaultSubobject<UWitTtsService>(TEXT("Service"));
	WitTtsService->MemoryCache = CreateDefaultSubobject<UTtsMemoryCache>(TEXT("MemoryCache"));
	WitTtsService->StorageCache = CreateDefaultSubobject<UTtsStorageCache>(TEXT("StorageCache"));
	
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
}

/**
 * Called when play begins
 */
void AWitTtsSpeaker::BeginPlay()
{
	Super::BeginPlay();

	if (WitTtsService != nullptr)
	{
		WitTtsService->OnSynthesizeResponse.AddUniqueDynamic(this, &AWitTtsSpeaker::OnSynthesizeResponse);
	}
}

/**
 * Called when the component is destroyed
 */
void AWitTtsSpeaker::BeginDestroy()
{
	Super::BeginDestroy();

	if (WitTtsService != nullptr)
	{
		WitTtsService->OnSynthesizeResponse.RemoveDynamic(this, &AWitTtsSpeaker::OnSynthesizeResponse);
	}
}

/**
 * Speak a phrase with the default configuration
 *
 * @param TextToSpeak [in] the text to speak
 */
void AWitTtsSpeaker::Speak(const FString& TextToSpeak)
{
	if (WitTtsService != nullptr)
	{
		WitTtsService->ConvertTextToSpeech(TextToSpeak);
	}
}


/**
 * Speak a phrase with custom settings
 *
 * @param ClipSettings [in] the settings to use
 */
void AWitTtsSpeaker::SpeakWithSettings(const FTtsConfiguration& ClipSettings)
{
	if (WitTtsService != nullptr)
	{
		WitTtsService->ConvertTextToSpeechWithSettings(ClipSettings);
	}
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

	if (AudioComponent->IsPlaying())
	{
		return;
	}
	
	AudioComponent->SetSound(SoundWave);
	AudioComponent->Play();
}
