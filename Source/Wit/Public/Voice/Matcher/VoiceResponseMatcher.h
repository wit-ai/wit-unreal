/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Wit/Request/WitPartialResponseValidatorBase.h"
#include "Wit/Request/WitResponse.h"
#include "VoiceResponseMatcher.generated.h"

/**
 * Base class for all response matchers. Implements shared functionality
 */
UCLASS()
class WIT_API UVoiceResponseMatcher : public UActorComponent
{
	GENERATED_BODY()

public:

	/**
	 * Default constructor
	 */
	UVoiceResponseMatcher();

	/**
	 * The tag of the voice experience to use in case there is more than one. If the tag is empty use the first found
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Experience")
	FName VoiceExperienceTag{};

	/** 
	 * Should this matcher also consider partial responses?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PartialResponse")
	bool bIsAlsoUsedForPartialResponse{false};

	/** 
	 * Should we auto accept once the intent and confidence score are matched?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PartialResponse")
	bool bAutoAcceptPartialResponseOncePastMatcherCriteria{false};

	/** 
	 * PartialResponseValidator class which will be used to validate PartialResponse. Check IWitPartialResponseValidator for details.
	 * Once the PartialResponseValidator's ValidatePartialResponse returns true, this method will auto accept it and cancel the request.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PartialResponse")
	TSubclassOf<UWitPartialResponseValidatorBase> PartialResponseValidator;
	
	/** 
	 * The PartialResponseValidator instance which is created from the PartialResponseValidator class.
	 */
	UPROPERTY()
	UWitPartialResponseValidatorBase* PartialResponseValidatorInstance;

	/**
	 * Callback that is called when a Wit.ai response is received which is used to see if we have a match
	 * 
	 * @param bIsSuccessful [in] true if the response was successful
	 * @param Response [in] the full response as a UStruct
	 */
	UFUNCTION()
	virtual void OnWitResponse(const bool bIsSuccessful, const FWitResponse& Response) {};

protected:
	
	/** Called when play is started */
	virtual void BeginPlay() override;

	/** Called to check and response to partial responses */
	void AcceptPartialResponse(const FWitResponse& Response);

};
