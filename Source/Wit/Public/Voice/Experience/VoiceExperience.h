/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Voice/Service/VoiceService.h"
#include "Voice/Events//VoiceEvents.h"
#include "VoiceExperience.generated.h"

/**
 * The base class of VoiceExperience
 */
UCLASS(ClassGroup=(Meta), Abstract)
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UVoiceEvents* VoiceEvents{};

	/**
	 * The configuration that will be used 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience")
	UWitAppConfigurationAsset* Configuration{};

	/**
	 * IVoiceService overrides
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool ActivateVoiceInput() override;
	
	UFUNCTION(BlueprintCallable)
	virtual bool ActivateVoiceInputWithRequestOptions(const FString& RequestOptions) override;

	UFUNCTION(BlueprintCallable)
	virtual bool ActivateVoiceInputImmediately() override;
	
	UFUNCTION(BlueprintCallable)
	virtual bool ActivateVoiceInputImmediatelyWithRequestOptions(const FString& RequestOptions) override;

	UFUNCTION(BlueprintCallable)
	virtual bool DeactivateVoiceInput() override;
	
	UFUNCTION(BlueprintCallable)
	virtual bool DeactivateAndAbortRequest() override;

	UFUNCTION(BlueprintCallable)
	virtual bool IsVoiceInputActive() const override;

	UFUNCTION(BlueprintCallable)
	virtual float GetVoiceInputVolume() const override;

	UFUNCTION(BlueprintCallable)
	virtual bool IsVoiceStreamingActive() const override;

	UFUNCTION(BlueprintCallable)
	virtual bool IsRequestInProgress() const override;

	UFUNCTION(BlueprintCallable)
	virtual void SendTranscription(const FString& Text) override;

	UFUNCTION(BlueprintCallable)
	virtual void SendTranscriptionWithRequestOptions(const FString& Text, const FString& RequestOptions) override;

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	virtual void AcceptPartialResponseAndCancelRequest(const FWitResponse& Response) const override;

protected:

	virtual void BeginPlay() override;
	
};
