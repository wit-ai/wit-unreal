/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Composer/Data/ComposerContextMap.h"

/**
 * Set the underlying Json object
 */
void UComposerContextMap::SetJsonObject(const TSharedPtr<FJsonObject> JsonObjectToUse)
{
	if (JsonObject == JsonObjectToUse)
	{
		return;
	}

	JsonObject = JsonObjectToUse;
	NestedContextMaps.Empty();
}

/**
 * Is there a field with the give name?
 */
bool UComposerContextMap::HasField(const FString& FieldName) const
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	return JsonObject->HasField(FieldName);
}

/**
 * Access a string field
 */
bool UComposerContextMap::GetStringField(const FString& FieldName, FString& Value) const
{
	if (!JsonObject.IsValid())
	{
		return false;
	}
	
	return JsonObject->TryGetStringField(FieldName, Value);
}

/**
 * Access an object field
 */
bool UComposerContextMap::GetObjectField(const FString& FieldName, UComposerContextMap*& Value)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}
	
	if (!JsonObject->HasTypedField<EJson::Object>(FieldName))
	{
		return false;
	}
	
	if (NestedContextMaps.Contains(FieldName))
	{
		Value = NestedContextMaps[FieldName];
	
		return true;
	}
	
	UComposerContextMap* NestedContextMap = NewObject<UComposerContextMap>();
	NestedContextMap->JsonObject = JsonObject->GetObjectField(FieldName);
	
	NestedContextMaps.Add(FieldName, NestedContextMap);

	Value = NestedContextMap;

	return true;
}

/**
 * Access a single object in an object array
 */
bool UComposerContextMap::GetObjectFromArrayField(const FString& FieldName, const int ArrayIndex, UComposerContextMap*& Value)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	if (!JsonObject->HasTypedField<EJson::Array>(FieldName))
	{
		return false;
	}

	// Aggregate a field name from the array name + index so we can look up entries uniquely

	const FString AggregateFieldName = FieldName + "[" + FString::FromInt(ArrayIndex) + "]";
	
	if (NestedContextMaps.Contains(AggregateFieldName))
	{
		Value = NestedContextMaps[AggregateFieldName];

		return true;
	}
	
	const TArray<TSharedPtr<FJsonValue>> JsonArray = JsonObject->GetArrayField(FieldName);
	const bool bIsArrayIndexInRange = ArrayIndex >= 0 && ArrayIndex < JsonArray.Num();
	
	if (!bIsArrayIndexInRange)
	{
		return false;
	}
	
	const TSharedPtr<FJsonValue> JsonValue = JsonArray[ArrayIndex];
	const TSharedPtr<FJsonObject>* NestedJsonObject;
	
	const bool bIsObject = JsonValue->TryGetObject(NestedJsonObject);

	if (!bIsObject)
	{
		return false;
	}
	
	UComposerContextMap* NestedContextMap = NewObject<UComposerContextMap>();

	NestedContextMap->JsonObject = *NestedJsonObject;
	
	NestedContextMaps.Add(AggregateFieldName, NestedContextMap);

	Value = NestedContextMap;

	return true;
}

/**
 * Access a string array field
 */
bool UComposerContextMap::GetStringArrayField(const FString& FieldName, TArray<FString>& Values) const
{
	if (!JsonObject.IsValid())
	{
		return false;
	}
	
	return JsonObject->TryGetStringArrayField(FieldName, Values);
}
