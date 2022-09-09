/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "ComposerContextMap.generated.h"

/**
 * Wrapper for accessing the context map JSON object
 */
UCLASS(NotBlueprintable)
class WIT_API UComposerContextMap final : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Set the underlying Json object
	 */
	void SetJsonObject(const TSharedPtr<FJsonObject> JsonObjectToUse);
	
	/**
	 * Access the underlying Json object
	 */
	TSharedPtr<FJsonObject> GetJsonObject() const { return JsonObject; }

	/**
	 * Is there a field with the give name?
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|ContextMap")
	UPARAM(DisplayName = "Is Found")bool HasField(const FString& FieldName) const;
	
	/**
	 * Access a string field. This can be used for any kind of value field (number, bool or string)
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|ContextMap")
	UPARAM(DisplayName = "Is Found")bool GetStringField(const FString& FieldName, UPARAM(DisplayName = "Out Value")FString& Value) const;

	/**
	 * Access an object field
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|ContextMap")
	UPARAM(DisplayName = "Is Found")bool GetObjectField(const FString& FieldName, UPARAM(DisplayName = "Out Value")UComposerContextMap*& Value);

	/**
	 * Access a single object in an object array
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|ContextMap")
	UPARAM(DisplayName = "Is Found")bool GetObjectFromArrayField(const FString& FieldName, const int ArrayIndex, UPARAM(DisplayName = "Out Value")UComposerContextMap*& Value);

	/**
	 * Access a string array field. This can be used for any kind of array of values field (number, bool or string)
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|ContextMap")
	UPARAM(DisplayName = "Is Found")bool GetStringArrayField(const FString& FieldName, UPARAM(DisplayName = "Out Value")TArray<FString>& Values) const;

private:

	TSharedPtr<FJsonObject> JsonObject{};

	UPROPERTY(Transient)
	TMap<FString, UComposerContextMap*> NestedContextMaps{};
	
};
