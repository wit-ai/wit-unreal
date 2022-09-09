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
 * Wrapper for accessing the context map JSON object. It caches nested Json objects as UObjects so they can be used
 * easily in blueprints
 */
UCLASS(NotBlueprintable)
class WIT_API UComposerContextMap final : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Set the underlying Json object
	 *
	 * @param JsonObjectToUse [in] the underlying Json object to use
	 */
	void SetJsonObject(const TSharedPtr<FJsonObject> JsonObjectToUse);
	
	/**
	 * Access the underlying Json object
	 */
	TSharedPtr<FJsonObject> GetJsonObject() const { return JsonObject; }

	/**
	 * Is there a field with the give name?
	 *
	 * @return true if the field was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|ContextMap")
	UPARAM(DisplayName = "Is Found")bool HasField(const FString& FieldName) const;
	
	/**
	 * Get a named string field. This can be used for any kind of value field (number, bool or string)
	 *
	 * @param FieldName [in] the field name to look for
	 * @param Value [out] the field value if found
	 *
	 * @return true if the field was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|ContextMap")
	UPARAM(DisplayName = "Is Found")bool GetStringField(const FString& FieldName, UPARAM(DisplayName = "Out Value")FString& Value) const;

	/**
	 * Get a named object field
	 *
	 * @param FieldName [in] the field name to look for
	 * @param Value [out] the field value if found
	 *
	 * @return true if the field was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|ContextMap")
	UPARAM(DisplayName = "Is Found")bool GetObjectField(const FString& FieldName, UPARAM(DisplayName = "Out Value")UComposerContextMap*& Value);

	/**
	 * Get a specific object field in a named object array
	 *
	 * @param FieldName [in] the field name to look for
	 * @param ArrayIndex [in] the array index in the array to retrieve
	 * @param Value [out] the field value if found
	 *
	 * @return true if the field was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|ContextMap")
	UPARAM(DisplayName = "Is Found")bool GetObjectFromArrayField(const FString& FieldName, const int ArrayIndex, UPARAM(DisplayName = "Out Value")UComposerContextMap*& Value);

	/**
	 * Get a named string array. This can be used for any kind of array of values field (number, bool or string) but not objects
	 *
	 * @param FieldName [in] the field name to look for
	 * @param Values [out] the field value if found
	 *
	 * @return true if the field was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|ContextMap")
	UPARAM(DisplayName = "Is Found")bool GetStringArrayField(const FString& FieldName, UPARAM(DisplayName = "Out Value")TArray<FString>& Values) const;

private:

	/** The underlying Json object */
	TSharedPtr<FJsonObject> JsonObject{};

	/** A cache of nested context maps. This is only lazily filled when a nested object field is accessed */
	UPROPERTY(Transient)
	TMap<FString, UComposerContextMap*> NestedContextMaps{};
	
};
