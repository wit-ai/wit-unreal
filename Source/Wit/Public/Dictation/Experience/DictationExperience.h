/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Dictation/Configuration/DictationConfiguration.h"
#include "GameFramework/Actor.h"
#include "Dictation/Service/DictationService.h"
#include "Dictation/Events/DictationEvents.h"
#include "DictationExperience.generated.h"

/**
 * The base class of dictation experiences
 */
UCLASS(ClassGroup=(Meta), Abstract, NotBlueprintable)
class WIT_API ADictationExperience : public AActor, public IDictationServiceBase
{
	GENERATED_BODY()
	
public:

	/**
	 * Sets default values for this actor's properties
	 */
	ADictationExperience();

	/**
	 * The configuration that will be used by the dictation service
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dictation")
	FDictationConfiguration Configuration{};

	/**
	 * The underlying dictation service that implements the dictation functionality
	 */
	UPROPERTY(Transient)
	UDictationService* DictationService{};
	
	/**
	 * The events used by the dictation service
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dictation")
	UDictationEvents* DictationEvents{};
	
	/**
	 * IDictationService overrides
	 */
	UFUNCTION(BlueprintCallable, Category = "Dictation")
	virtual bool ActivateDictation() override;
	
	UFUNCTION(BlueprintCallable, Category = "Dictation")
	virtual bool ActivateDictationWithRequestOptions(const FString& RequestOptions) override;

	UFUNCTION(BlueprintCallable, Category = "Dictation")
	virtual bool ActivateDictationImmediately() override;

	UFUNCTION(BlueprintCallable, Category = "Dictation")
	virtual bool DeactivateDictation() override;

	UFUNCTION(BlueprintCallable, Category = "Dictation")
	virtual bool IsDictationActive() const override;
	
	UFUNCTION(BlueprintCallable, Category = "Dictation")
	virtual bool IsRequestInProgress() const override;

protected:

	virtual void BeginPlay() override;
	
};
