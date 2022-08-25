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
	 * The events used by the voice service
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voice Experience")
	UVoiceEvents* VoiceEvents{};

	/**
	 * The configuration that will be used 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice Experience")
	UWitAppConfigurationAsset* Configuration{};

	/**
	 * IVoiceService overrides
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual bool ActivateVoiceInput() override;
	
	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual bool ActivateVoiceInputWithRequestOptions(const FString& RequestOptions) override;

	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual bool ActivateVoiceInputImmediately() override;
	
	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual bool ActivateVoiceInputImmediatelyWithRequestOptions(const FString& RequestOptions) override;

	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual bool DeactivateVoiceInput() override;
	
	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual bool DeactivateAndAbortRequest() override;

	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual bool IsVoiceInputActive() const override;

	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual float GetVoiceInputVolume() const override;

	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual bool IsVoiceStreamingActive() const override;

	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual bool IsRequestInProgress() const override;

	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual void SendTranscription(const FString& Text) override;

	UFUNCTION(BlueprintCallable, Category = "Voice Experience")
	virtual void SendTranscriptionWithRequestOptions(const FString& Text, const FString& RequestOptions) override;

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Voice Experience")
	virtual void AcceptPartialResponseAndCancelRequest(const FWitResponse& Response) const override;

protected:

	virtual void BeginPlay() override;
	
};
