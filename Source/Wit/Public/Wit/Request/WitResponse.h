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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Name{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	int64 Id{0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	float Confidence{0.0f};
};

/**
 * UStruct representation of the JSON entity interval object used by Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitEntityInterval
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Value{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Unit{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Grain{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Product{};
};

/**
 * UStruct representation of the JSON entity normalized object used by Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitEntityNormalized
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Value{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Unit{};
};

/**
 * UStruct representation of the JSON entity value object used by Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitEntityValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Value{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Type{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Grain{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FWitEntityInterval From{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FWitEntityInterval To{};
};

/**
 * UStruct representation of the JSON entity object used by Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitEntity
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Value{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Name{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	int64 Id{0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Role{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Body{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	float Confidence{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Type{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Unit{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Grain{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	int32 Start{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	int32 End{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FWitEntityInterval From{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FWitEntityInterval To{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FWitEntityNormalized Normalized{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TArray<FWitEntityValue> Values{};
};

/**
 * UStruct representation of the JSON trait object used by Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitTrait
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Value{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	int64 Id{0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Text{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TArray<FWitIntent> Intents{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TMap<FString, FWitEntity> Entities{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TMap<FString, FWitTrait> Traits{};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Voice Experience")
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Name{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Locale{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Gender{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TArray<FWitTtsVoice> En_US{};
};
