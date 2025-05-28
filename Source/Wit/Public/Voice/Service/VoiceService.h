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
#include "Interface/IVoiceServiceBase.h"
#include "Wit/Request/WitResponse.h"
#include "Voice/Events/VoiceEvents.h"
#include "VoiceService.generated.h"

/**
 * Base class implementation of a voice service
 */
UCLASS(ClassGroup=(Meta), Abstract)
class WIT_API UVoiceService : public UActorComponent, public IVoiceServiceBase
{
	GENERATED_BODY()

public:
	
	UVoiceService() : Super() {};
	
	/**
	 * Set the configuration to use with this service
	 */
	virtual void SetConfiguration(UWitAppConfigurationAsset* ConfigurationToUse, bool bUseWebSocketToUse)
	{	
		Configuration = ConfigurationToUse;
		bUseWebSocket = bUseWebSocketToUse;
	}

	/**
	 * Set the events to use with this service
	 */
	void SetEvents(UVoiceEvents* EventsToUse) { Events = EventsToUse; }
	
	/**
	 * IVoiceService overrides
	 */
	virtual bool ActivateVoiceInput() override { return false; }
	virtual bool ActivateVoiceInputWithRequestOptions(const FString& RequestOptions)override { return false; }
	virtual bool ActivateVoiceInputImmediately() override { return false; }
	virtual bool ActivateVoiceInputImmediatelyWithRequestOptions(const FString& RequestOptions) override { return false; }
	virtual bool DeactivateVoiceInput() override { return false; }
	virtual bool DeactivateAndAbortRequest() override{ return false; }
	virtual bool IsVoiceInputActive() const override { return false; }
	virtual float GetVoiceInputVolume() const override { return 0.0; }
	virtual bool IsVoiceStreamingActive() const override { return false; }
	virtual bool IsRequestInProgress() const override { return false; }
	virtual void SendTranscription(const FString& Text) override {}
	virtual void SendTranscriptionWithRequestOptions(const FString& Text, const FString& RequestOptions) override {}
	virtual void AcceptPartialResponseAndCancelRequest(const FWitResponse& Response) override {}

protected:

	/**
	 * The configuration that this service should use
	 */
	UPROPERTY(Transient)
	UWitAppConfigurationAsset* Configuration{};

	/**
	* Whether or not streaming is enabled on the call to Wit.ai
	*/
	UPROPERTY(Transient)
	bool bUseWebSocket{false};
	
	/**
	 * The events that this service should use in callbacks
	 */
	UPROPERTY(Transient)
	UVoiceEvents* Events{};

};
