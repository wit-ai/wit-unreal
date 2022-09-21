/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Dictation/Service/DictationService.h"
#include "Voice/Experience/VoiceExperience.h"
#include "WitDictationService.generated.h"

/**
 * Component that encapsulates the Wit Text to Speech API. Provides functionality for speech synthesis from text input
 * using Wit.ai. To use it simply attach the UWitTtsService component in the hierarchy of any Actor
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

	/** Callback to redirect the speech and message endpoints to the composer equivalents */
	void OnDictationRequestCustomize(FWitRequestConfiguration& RequestConfiguration);

	/** Callbacks that we will connect to the voice events */

	UFUNCTION()
	void OnFullTranscripton(const FString& Transcription);

	UFUNCTION()
	void OnPartialTranscripton(const FString& Transcription);

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

};
