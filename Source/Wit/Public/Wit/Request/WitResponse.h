/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WitResponse.generated.h"

/**
 * UStruct representation of the JSON intent object used by Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitIntent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 Id{0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Confidence{0.0f};
};

/**
 * UStruct representation of the JSON entity interval object used by Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitEntityInterval
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Unit{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Grain{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Product{};
};

/**
 * UStruct representation of the JSON entity normalized object used by Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitEntityNormalized
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Unit{};
};

/**
 * UStruct representation of the JSON entity value object used by Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitEntityValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Type{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Grain{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWitEntityInterval From{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWitEntityInterval To{};
};

/**
 * UStruct representation of the JSON entity object used by Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitEntity
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 Id{0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Role{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Body{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Confidence{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Type{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Unit{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Grain{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Start{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 End{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWitEntityInterval From{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWitEntityInterval To{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWitEntityNormalized Normalized{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWitEntityValue> Values{};
};

/**
 * UStruct representation of the JSON trait object used by Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitTrait
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Value{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 Id{0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Confidence{0.0f};	
};

/**
 * UStruct representation of the full JSON object used by Wit.ai responses. See the Wit.ai
 * documentation for the meaning of each specific field.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitResponse
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Text{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWitIntent> Intents{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FWitEntity> Entities{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FWitTrait> Traits{};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool Is_Final{false};

	/**
	 * Reset the response to its defaults
	 */
	void Reset();
};

/**
 * UStruct representation of the JSON voice preset object
 */
USTRUCT(BlueprintType)
struct WIT_API FWitTtsVoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Locale{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Gender{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Styles{};
};

/**
 * UStruct representation of the full JSON object used by Wit.ai /voices response. See the Wit.ai
 * documentation for the meaning of each specific field.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitTtsVoicesResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWitTtsVoice> En_US{};
};
