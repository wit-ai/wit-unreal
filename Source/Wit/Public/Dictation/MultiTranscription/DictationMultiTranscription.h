/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dictation/Experience/DictationExperience.h"
#include "Voice/Events/VoiceEvents.h"
#include "DictationMultiTranscription.generated.h"

/**
 * Simple component for handling longer transcriptions across multiple requests
 */
UCLASS( ClassGroup=(Meta), meta=(BlueprintSpawnableComponent) )
class WIT_API UDictationMultiTranscription final : public UActorComponent
{
	GENERATED_BODY()

public:

	/** The tag of the dictation experience to use in case there is more than one. If the tag is empty use the first found */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	FName DictationExperienceTag{};

	/** Whether we use the max activations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bShouldUseLimit{true};
	
	/** The maximum number of activations before we stop adding more. This limits the max string length and requires a clear after */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int MaxActivations{5};

	/** This forces a clear after we reach the max number of activations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoClearAfterMaxActivations{false};
	
	/** Number of lines to insert between activations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int LinesBetweenActivations{2};

	/** A string separator to insert between activations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString ActivationSeparator{};

	/**
	 * Clear the text and start again
	 */
	UFUNCTION(BlueprintCallable, Category = "Dictation|Multi")
	void Clear();
		
	/**
	 * Callback to call whenever the transcription is updated
	 */
	UPROPERTY(BlueprintAssignable)
	FOnWitTranscriptionDelegate OnTranscriptionUpdated{};

protected:

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	
private:

	void DoUpdateTranscription(const FString& PartialTranscription);
	void AppendSeparator(FString& AppendTo);

	/** Callbacks that we will connect to the voice events */

	UFUNCTION()
	void OnFullTranscription(const FString& FullTranscription);

	UFUNCTION()
	void OnPartialTranscription(const FString& PartialTranscription);

	/** The current full text */
	UPROPERTY(VisibleInstanceOnly, Category = "Dictation|Multi")
	FString Text{};
	
	/** The current full text */
	UPROPERTY(VisibleInstanceOnly, Category = "Dictation|Multi")
	int ActivationCount{};
	
	/**
	 * The dictation experience that we will use
	 */
	UPROPERTY(Transient)
	ADictationExperience* DictationExperience{};

};
