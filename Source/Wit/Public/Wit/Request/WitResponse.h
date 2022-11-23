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
 * Representation of the JSON intent object used by Wit.ai. See the Wit.ai
 * documentation for the meaning of each specific field.
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
 * Representation of the JSON entity interval object used by Wit.ai. See the Wit.ai
 * documentation for the meaning of each specific field.
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
 * Representation of the JSON entity normalized object used by Wit.ai. See the Wit.ai
 * documentation for the meaning of each specific field.
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
 * Representation of the JSON entity value object used by Wit.ai. See the Wit.ai
 * documentation for the meaning of each specific field.
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
 * Representation of the JSON entity object used by Wit.ai. See the Wit.ai
 * documentation for the meaning of each specific field.
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
 * Representation of the JSON trait object used by Wit.ai. See the Wit.ai
 * documentation for the meaning of each specific field.
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


USTRUCT(BlueprintType)
struct WIT_API FWitEntities
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Name{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TArray<FWitEntity> Entities;

};

/**
 * Representation of the full JSON object used by Wit.ai responses. See the Wit.ai
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

	/**
	 * Technically, each key can have multi Entities.
	 * But to simplify this, and also because Unreal does not support Nested structure,
	 * The value - FWitEntity only contains the very first Entity.
	 * This works for most of cases.
	 *
	 * If you are looking for all entities, then please check AllEntities. 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TMap<FString, FWitEntity> Entities{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TMap<FString, FWitEntities> AllEntities{};

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
 * Representation of the JSON voice preset object. See the Wit.ai
 * documentation for the meaning of each specific field.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitVoiceDefinition
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
 * Representation of the full JSON object used by Wit.ai /voices response. See the Wit.ai
 * documentation for the meaning of each specific field.
 */
USTRUCT(BlueprintType)
struct WIT_API FWitVoicesResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TArray<FWitVoiceDefinition> En_US{};
};

/**
 * Representation of the full JSON object returned by Wit.ai composer
 */
USTRUCT(BlueprintType)
struct WIT_API FWitComposerResponse
{
	GENERATED_BODY()

	/** Is input expected to follow? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer")
	bool Expects_Input{false};

	/** String identifier of the action to perform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer")
	FString Action{};

	/** The full speech response */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composer")
	FWitResponse Response{};
};

/**
 * Representation of the JSON entity object used by /intents endpoint
 */
USTRUCT(BlueprintType)
struct WIT_API FWitEntityShortDefinition
{
	GENERATED_BODY()

	/** The name of the entity as defined in Wit.ai */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Name{};

	/** The entity's unique id */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	int64 Id{0};
};

/**
 * Representation of the JSON entity object used by Wit.ai /entities endpoint 
 */
USTRUCT(BlueprintType)
struct WIT_API FWitEntityDefinition
{
	GENERATED_BODY()

	/** The name of the entity as defined in Wit.ai */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Name{};

	/** The entity's unique id */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	int64 Id{0};

	/** The lookups associated with this entity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TArray<FString> Lookups{};

	/** The roles associated with this entity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TArray<FString> Roles{};
};

/**
 * Representation of the JSON intent definition object returned by Wit.ai /intents endpoint
 */
USTRUCT(BlueprintType)
struct WIT_API FWitIntentDefinition
{
	GENERATED_BODY()

	/** The name of the intent as defined in Wit.ai */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Name{};

	/** The intent's unique id */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	int64 Id{0};

	/** The entities associated with this intent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TArray<FWitEntityShortDefinition> Entities{};
};

/**
 * Representation of the JSON trait value object used by Wit.ai /traits endpoint
 */
USTRUCT(BlueprintType)
struct WIT_API FWitTraitValueDefinition
{
	GENERATED_BODY()

	/** The name of the trait value as defined in Wit.ai */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Name{};

	/** The value of the trait */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Value{};
};

/**
 * Representation of the JSON trait definition object returned by Wit.ai /traits endpoint
 */
USTRUCT(BlueprintType)
struct WIT_API FWitTraitDefinition
{
	GENERATED_BODY()

	/** The name of the trait as defined in Wit.ai */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Name{};

	/** The trait's unique id */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	int64 Id{0};

	/** The values associated with the trait */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	TArray<FWitTraitValueDefinition> Values{};
};

/**
 * Representation of the JSON app definition object returned by Wit.ai /apps endpoint
 */
USTRUCT(BlueprintType)
struct WIT_API FWitAppDefinition
{
	GENERATED_BODY()

	/** The name of the app as defined in Wit.ai */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Name{};

	/** The app's unique id */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Id{};

	/** The language the app is defined in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Lang{};

	/** Unused */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	bool Private{};

	/** The timestamp when the app was created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	FString Created_At{};

	/** If this is the app associated with the token that was supplied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Experience")
	bool Is_App_For_Token{};
};
