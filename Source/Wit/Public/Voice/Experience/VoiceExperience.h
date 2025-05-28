/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Voice/Service/VoiceService.h"
#include "Voice/Events//VoiceEvents.h"
#include "VoiceExperience.generated.h"


/**
 * The base class of VoiceExperience
 */
UCLASS(ClassGroup=(Meta), Abstract, NotBlueprintable)
class WIT_API AVoiceExperience : public AActor, public IVoiceServiceBase
{
	GENERATED_BODY()
	
public:

	/**
	 * Sets default values for this actor's properties
	 */
	AVoiceExperience();

	/**
	 * The underlying voice service that implements the voice functionality
	 */
	UPROPERTY(Transient)
	UVoiceService* VoiceService{};
		
	/**
	 * The configuration that will be used 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice")
	UWitAppConfigurationAsset* Configuration{};

	/**
	* Should the request use WebSocket connection to Wit.ai
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (EditCondition = "false", EditConditionHides))
	bool bUseWebSocket{false};

	/**
	 * The events used by the voice service
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voice")
	UVoiceEvents* VoiceEvents{};

	/**
	 * IVoiceService overrides
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual bool ActivateVoiceInput() override;
	
	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual bool ActivateVoiceInputWithRequestOptions(const FString& RequestOptions) override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual bool ActivateVoiceInputImmediately() override;
	
	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual bool ActivateVoiceInputImmediatelyWithRequestOptions(const FString& RequestOptions) override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual bool DeactivateVoiceInput() override;
	
	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual bool DeactivateAndAbortRequest() override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual bool IsVoiceInputActive() const override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual float GetVoiceInputVolume() const override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual bool IsVoiceStreamingActive() const override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual bool IsRequestInProgress() const override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual void SendTranscription(const FString& Text) override;

	UFUNCTION(BlueprintCallable, Category = "Voice")
	virtual void SendTranscriptionWithRequestOptions(const FString& Text, const FString& RequestOptions) override;

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Voice")
	virtual void AcceptPartialResponseAndCancelRequest(const FWitResponse& Response) override;

protected:

	virtual void BeginPlay() override;
	
};
