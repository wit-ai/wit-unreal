/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Composer/Data/ComposerContextMap.h"
#include "Dom/JsonObject.h"

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
 * Clear the underlying Json object
 */
void UComposerContextMap::Reset()
{
	JsonObject.Reset();
	JsonObject = MakeShared<FJsonObject>();
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
 * Get a named integer field
 *
 * @param FieldName [in] the field name to look for
 * @param Value [out] the field value if found
 *
 * @return true if the field was found
 */
bool UComposerContextMap::GetIntegerField(const FString& FieldName, int& Value) const
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	return JsonObject->TryGetNumberField(FieldName, Value);
}

/**
 * Get a named number field
 *
 * @param FieldName [in] the field name to look for
 * @param Value [out] the field value if found
 *
 * @return true if the field was found
 */
bool UComposerContextMap::GetNumberField(const FString& FieldName, float& Value) const
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	double DoubleValue = 0.0f;
	const bool bIsFound = JsonObject->TryGetNumberField(FieldName, DoubleValue);

	if (bIsFound)
	{
		Value = static_cast<float>(DoubleValue);
	}
	
	return bIsFound;
}

/**
 * Get a named string field. This can be used for any kind of value field (number, bool or string)
 *
 * @param FieldName [in] the field name to look for
 * @param Value [out] the field value if found
 *
 * @return true if the field was found
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
 * Get a named object field
 *
 * @param FieldName [in] the field name to look for
 * @param Value [out] the field value if found
 *
 * @return true if the field was found
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
 * Get a specific object field in a named object array
 *
 * @param FieldName [in] the field name to look for
 * @param ArrayIndex [in] the array index in the array to retrieve
 * @param Value [out] the field value if found
 *
 * @return true if the field was found
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
 * Get a named string array. This can be used for any kind of array of values field (number, bool or string) but not objects
 *
 * @param FieldName [in] the field name to look for
 * @param Values [out] the field value if found
 *
 * @return true if the field was found
 */
bool UComposerContextMap::GetStringArrayField(const FString& FieldName, TArray<FString>& Values) const
{
	if (!JsonObject.IsValid())
	{
		return false;
	}
	
	return JsonObject->TryGetStringArrayField(FieldName, Values);
}

/**
 * Set a named integer field
 *
 * @param FieldName [in] the field name to look for
 * @param Value [in] the field value to set
 */
void UComposerContextMap::SetIntegerField(const FString& FieldName, const int Value)
{
	if (!JsonObject.IsValid())
	{
		return;
	}

	JsonObject->SetNumberField(FieldName, Value);
}

/**
 * Set a named float field
 *
 * @param FieldName [in] the field name to look for
 * @param Value [in] the field value to set
 */
void UComposerContextMap::SetNumberField(const FString& FieldName, const float Value)
{
	if (!JsonObject.IsValid())
	{
		return;
	}

	JsonObject->SetNumberField(FieldName, Value);
}

/**
 * Set a named string field. This can be used for any kind of value field (number, bool or string)
 *
 * @param FieldName [in] the field name to look for
 * @param Value [in] the field value to set
 */
void UComposerContextMap::SetStringField(const FString& FieldName, const FString& Value)
{
	if (!JsonObject.IsValid())
	{
		return;
	}

	JsonObject->SetStringField(FieldName, Value);
}

/**
 * Set a named object field
 *
 * @param FieldName [in] the field name to look for
 * @param Value [in] the field value to set
 */
void UComposerContextMap::SetObjectField(const FString& FieldName, UComposerContextMap* Value)
{
	if (!JsonObject.IsValid())
	{
		return;
	}

	JsonObject->SetObjectField(FieldName, Value->GetJsonObject());
}
