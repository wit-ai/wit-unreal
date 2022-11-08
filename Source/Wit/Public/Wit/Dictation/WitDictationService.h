/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Dictation/Configuration/DictationConfiguration.h"
#include "Dictation/Service/DictationService.h"
#include "Voice/Experience/VoiceExperience.h"
#include "WitDictationService.generated.h"

/**
 * Component that encapsulates the Wit dictation API. Provides functionality for long form speech to text
 * using Wit.ai
 */
UCLASS( ClassGroup=(Meta), meta=(BlueprintSpawnableComponent) )
class WIT_API UWitDictationService final : public UDictationService
{
	GENERATED_BODY()

public:
	
	/**
	 * Default constructor
	 */
	UWitDictationService();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * IDictationService overrides
	 */
	virtual bool ActivateDictation() override;
	virtual bool ActivateDictationWithRequestOptions(const FString& RequestOptions) override;
	virtual bool ActivateDictationImmediately() override;
	virtual bool DeactivateDictation() override;
	virtual bool IsDictationActive() const override;
	virtual bool IsRequestInProgress() const override;

protected:
	
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

private:

	/** Callback to redirect the speech endpoint to the dictation equivalent */
	void OnDictationRequestCustomize(FWitRequestConfiguration& RequestConfiguration);

	/** Callbacks that we will connect to the voice events */

	UFUNCTION()
	void OnFullTranscription(const FString& Transcription);

	UFUNCTION()
	void OnPartialTranscription(const FString& Transcription);

	UFUNCTION()
	void OnStartVoiceInput();

	UFUNCTION()
	void OnStopVoiceInput();

	UFUNCTION()
	void OnWitResponse(const bool bIsSuccessful, const FWitResponse& WitResponse);

	UFUNCTION()
	void OnWitError(const FString& ErrorMessage, const FString& HumanReadableMessage);
	
	/**
	 * The voice experience that dictation will use
	 */
	UPROPERTY(Transient)
	AVoiceExperience* VoiceExperience{};

	/** Used to track how long since we activated voice input when capturing */
	float LastActivateTime{0.0f};

	/** Did a manual deactivation occur? */
	bool bWasManuallyDeactivated{false};
	
};
