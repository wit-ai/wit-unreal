/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Wit/Configuration/WitAppConfigurationAsset.h"
#include "Interface/ITtsServiceBase.h"
#include "TTS/Cache/Memory/TtsMemoryCacheHandler.h"
#include "TTS/Cache/Storage/TtsStorageCacheHandler.h"
#include "TTS/Configuration/TtsVoicePresetAsset.h"
#include "TTS/Events/TtsEvents.h"
#include "Wit/Request/WitRequestTypes.h"
#include "TtsService.generated.h"

/**
 * Base class implementation of a TTS service
 */
UCLASS(ClassGroup=(Meta), Abstract)
class WIT_API UTtsService : public UActorComponent, public ITtsServiceBase
{
	GENERATED_BODY()

public:
	
	UTtsService() : Super() {};

	/**
	 * Set the configuration to use for TTS
	 */
	virtual void SetConfiguration(
		UWitAppConfigurationAsset* ConfigurationToUse,
		UTtsVoicePresetAsset* VoicePresetToUse,
		EWitRequestAudioFormat AudioTypeToUse,
		bool bUseStreamingToUse,
		float InitialStreamBufferSizeToUse,
		bool bUseWebSocketToUse)
	{
		Configuration = ConfigurationToUse;
		VoicePreset = VoicePresetToUse;
		AudioType = AudioTypeToUse;
		bUseStreaming = bUseStreamingToUse;
		InitialStreamBufferSize = InitialStreamBufferSizeToUse;
		bUseWebSocket = bUseWebSocketToUse;
	}

	/**
	 * Set the handlers to use
	 */
	void SetHandlers(UTtsEvents* EventHandlerToUse, UTtsMemoryCacheHandler* MemoryCacheHandlerToUse, UTtsStorageCacheHandler* StorageCacheHandlerToUse)
	{
		EventHandler = EventHandlerToUse;
		MemoryCacheHandler = MemoryCacheHandlerToUse;
		StorageCacheHandler = StorageCacheHandlerToUse;
	}
	
	/**
	 * ITtsService overrides
	 */
	virtual bool IsRequestInProgress() const override { return false; }
	virtual void ConvertTextToSpeech(const FString& TextToConvert, bool bQueueAudio = true) override {}
	virtual void ConvertTextToSpeechWithSettings(const FTtsConfiguration& ClipSettings, bool bQueueAudio = true) override {}
	virtual void FetchAvailableVoices() override {}

protected:

	/**
	 * The configuration that this service should use
	 */
	UPROPERTY(Transient)
	UWitAppConfigurationAsset* Configuration{};

	/**
	 * The Wit TTS Voice Preset that will be used by Wit.ai
	 */
	UPROPERTY(Transient)
	UTtsVoicePresetAsset* VoicePreset{};

	/**
	 * The Wit TTS Audio Type that will be used by Wit.ai
	 */
	UPROPERTY(Transient)
	EWitRequestAudioFormat AudioType{EWitRequestAudioFormat::Wav};

	/**
	* Whether or not streaming is enabled on the call to Wit.ai
	*/
	UPROPERTY(Transient)
	bool bUseStreaming{false};

	/**
	* How many seconds of data to buffer before playing audio.
	*/
	UPROPERTY(Transient)
	float InitialStreamBufferSize{ 0.1f };

	/**
	* Whether or not streaming is enabled on the call to Wit.ai
	*/
	UPROPERTY(Transient)
	bool bUseWebSocket{false};

	/**
	 * The events that this service should use in callbacks
	 */
	UPROPERTY(Transient)
	UTtsEvents* EventHandler{};
	
	/**
	  * Memory cache to store converted voice clips as USoundWave
	  */
	UPROPERTY(Transient)
	UTtsMemoryCacheHandler* MemoryCacheHandler{};

	/**
	 * Storage cache to store converted voice clips as raw data on disk
	 */
	UPROPERTY(Transient)
	UTtsStorageCacheHandler* StorageCacheHandler{};

};
