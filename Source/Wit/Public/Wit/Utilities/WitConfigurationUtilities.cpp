/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitConfigurationUtilities.h"
#include "Engine/Engine.h"
#include "Wit/Request/WitRequestSubsystem.h"
#include "Wit/Request/WitRequestBuilder.h"
#include "Wit/Utilities/WitLog.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "JsonObjectConverter.h"

UWitAppConfigurationAsset* FWitConfigurationUtilities::Configuration = nullptr;

/**
 * Refresh all the associated app data for a configuration
 */
void FWitConfigurationUtilities::RefreshConfiguration(UWitAppConfigurationAsset* ConfigurationToRefresh)
{
	if (Configuration != nullptr)
	{
		UE_LOG(LogWit, Verbose, TEXT("RefreshConfiguration - Request already in progress"));
		return;
	}

	Configuration = ConfigurationToRefresh;

	// Clear out data

	UE_LOG(LogWit, Verbose, TEXT("RefreshConfiguration - Starting refresh"));
	
	Configuration->Application.Data.Application.Id = 0;
	Configuration->Application.Data.Application.Name = FString();
	Configuration->Application.Data.Application.Lang = FString();
	Configuration->Application.Data.Application.Private = false;
	Configuration->Application.Data.Application.Created_At = FString();
	Configuration->Application.Data.Application.Is_App_For_Token = false;

	Configuration->Application.Data.Intents.Empty();
	Configuration->Application.Data.Entities.Empty();
	Configuration->Application.Data.Traits.Empty();
	Configuration->Application.Data.Voices.Empty();

	RequestAppList();
}

/**
 * Requests the available list of apps from Wit.ai
 */
void FWitConfigurationUtilities::RequestAppList()
{
	FWitRequestConfiguration RequestConfiguration{};

	if (!SetupListRequest(RequestConfiguration, EWitRequestEndpoint::GetApps, true))
	{
		Configuration = nullptr;
		return;
	}

	FWitRequestBuilder::AddParameter(RequestConfiguration, EWitParameter::Offset, TEXT("0"));
	FWitRequestBuilder::AddParameter(RequestConfiguration, EWitParameter::Limit, TEXT("10"));

	RequestConfiguration.OnRequestError.AddStatic(&FWitConfigurationUtilities::OnAppsRequestError);
	RequestConfiguration.OnRequestComplete.AddStatic(&FWitConfigurationUtilities::OnAppsRequestComplete);

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	RequestSubsystem->BeginStreamRequest(RequestConfiguration);
	RequestSubsystem->EndStreamRequest();
}

/**
 * Called when a Wit apps request is successfully completed. The response will contain a list of available apps
 */
void FWitConfigurationUtilities::OnAppsRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	UE_LOG(LogWit, Verbose, TEXT("OnAppsRequestComplete - Final response size: %d"), BinaryResponse.Num());

	// We can't use the Json object because it will only contain the final entry of the array. We therefore use the binary data and convert it as an array

	const FUTF8ToTCHAR DataAsCharacters(reinterpret_cast<const ANSICHAR*>(BinaryResponse.GetData()), BinaryResponse.Num());
	const FString JsonArrayString = FString(DataAsCharacters.Length(), DataAsCharacters.Get());

	TArray<FWitAppDefinition> Applications;
	const bool bIsConversionError = !FJsonObjectConverter::JsonArrayStringToUStruct<FWitAppDefinition>(JsonArrayString, &Applications);

	if (bIsConversionError)
	{
		OnAppsRequestError(TEXT("Json To UStruct failed"), TEXT("Converting the Json response to a UStruct failed"));
		return;
	}

	UE_LOG(LogWit, Verbose, TEXT("OnAppsRequestComplete - Received apps: %d"), Applications.Num());

	// Now we have all the available apps we look for the one marked as matching our server token

	bool bIsAppFound = false;
	
	for (const auto& Application : Applications)
	{
		if (Application.Is_App_For_Token)
		{
			Configuration->Application.Data.Application = Application;

			RequestClientToken(Application.Id);

			bIsAppFound = true;
			
			break;
		}
	}

	if (!bIsAppFound)
	{
		Configuration = nullptr;
	}
}

/**
 * Called when a apps request errors
 */
void FWitConfigurationUtilities::OnAppsRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage)
{
	UE_LOG(LogWit, Warning, TEXT("OnAppsRequestError - request failed with error: %s - %s"), *ErrorMessage, *HumanReadableErrorMessage);

	// If the app request fails there is no point continuing as we won't have an app id to use
	
	Configuration = nullptr;
}

/**
 * Requests the client token for an app
 */
void FWitConfigurationUtilities::RequestClientToken(const FString& AppId)
{
	FWitRequestConfiguration RequestConfiguration{};

	if (!SetupListRequest(RequestConfiguration, EWitRequestEndpoint::ClientToken, true))
	{
		Configuration = nullptr;
		return;
	}

	// Fill in the resolved endpoint

	RequestConfiguration.Endpoint = FString::Format(TEXT("apps/{0}/client_tokens"), {AppId});

	RequestConfiguration.OnRequestError.AddStatic(&FWitConfigurationUtilities::OnClientTokenRequestError);
	RequestConfiguration.OnRequestComplete.AddStatic(&FWitConfigurationUtilities::OnClientTokenRequestComplete);

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	RequestSubsystem->BeginStreamRequest(RequestConfiguration);

	// Construct the body parameters. The only parameter currently is 'refresh'

	const TSharedPtr<FJsonObject> RequestBody = MakeShared<FJsonObject>();

	RequestBody->SetBoolField(TEXT("refresh"), false);

	RequestSubsystem->WriteJsonData(RequestBody.ToSharedRef());
	RequestSubsystem->EndStreamRequest();
}

/**
 * Called when a Wit client token request is successfully completed
 */
void FWitConfigurationUtilities::OnClientTokenRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	UE_LOG(LogWit, Verbose, TEXT("OnClientTokenRequestComplete - Final response size: %d"), BinaryResponse.Num());

	Configuration->Application.ClientAccessToken = JsonResponse->GetStringField(TEXT("client_token"));

	UE_LOG(LogWit, Verbose, TEXT("OnIntentsRequestComplete - Received client token: %s"), *Configuration->Application.ClientAccessToken);

	RequestIntentList();
}

/**
 * Called when a client token request errors
 */
void FWitConfigurationUtilities::OnClientTokenRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage)
{
	UE_LOG(LogWit, Warning, TEXT("OnClientTokenRequestError - request failed with error: %s - %s"), *ErrorMessage, *HumanReadableErrorMessage);

	// If the client token request fails there is no point continuing as we won't have a client token to use
	
	Configuration = nullptr;
}

/**
 * Requests the available list of intents for an app
 */
void FWitConfigurationUtilities::RequestIntentList()
{
	FWitRequestConfiguration RequestConfiguration{};

	if (!SetupListRequest(RequestConfiguration, EWitRequestEndpoint::GetIntents, false))
	{
		Configuration = nullptr;
		return;
	}

	RequestConfiguration.OnRequestError.AddStatic(&FWitConfigurationUtilities::OnIntentsRequestError);
	RequestConfiguration.OnRequestComplete.AddStatic(&FWitConfigurationUtilities::OnIntentsRequestComplete);

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	RequestSubsystem->BeginStreamRequest(RequestConfiguration);
	RequestSubsystem->EndStreamRequest();
}

/**
 * Called when a Wit intents request is successfully completed. The response will contain a list of available intents
 */
void FWitConfigurationUtilities::OnIntentsRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	UE_LOG(LogWit, Verbose, TEXT("OnIntentsRequestComplete - Final response size: %d"), BinaryResponse.Num());

	// We can't use the Json object because it will only contain the final entry of the array. We therefore use the binary data and convert it as an array
	// TODO - in Unity we actually grab the detailed intent data using /intents/<intent_name> 

	const FUTF8ToTCHAR DataAsCharacters(reinterpret_cast<const ANSICHAR*>(BinaryResponse.GetData()), BinaryResponse.Num());
	const FString JsonArrayString = FString(DataAsCharacters.Length(), DataAsCharacters.Get());

	const bool bIsConversionError = !FJsonObjectConverter::JsonArrayStringToUStruct<FWitIntentDefinition>( JsonArrayString, &Configuration->Application.Data.Intents);

	if (bIsConversionError)
	{
		OnIntentsRequestError(TEXT("Json To UStruct failed"), TEXT("Converting the Json response to a UStruct failed"));
		return;
	}

	UE_LOG(LogWit, Verbose, TEXT("OnIntentsRequestComplete - Received intents: %d"), Configuration->Application.Data.Intents.Num());

	RequestEntityList();
}

/**
 * Called when a intents request errors
 */
void FWitConfigurationUtilities::OnIntentsRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage)
{
	UE_LOG(LogWit, Warning, TEXT("OnIntentsRequestError - request failed with error: %s - %s"), *ErrorMessage, *HumanReadableErrorMessage);

	RequestEntityList();
}

/**
 * Requests the available list of entities for an app
 */
void FWitConfigurationUtilities::RequestEntityList()
{
	FWitRequestConfiguration RequestConfiguration{};

	if (!SetupListRequest(RequestConfiguration, EWitRequestEndpoint::GetEntities, true))
	{
		Configuration = nullptr;
		return;
	}

	RequestConfiguration.OnRequestError.AddStatic(&FWitConfigurationUtilities::OnEntitiesRequestError);
	RequestConfiguration.OnRequestComplete.AddStatic(&FWitConfigurationUtilities::OnEntitiesRequestComplete);

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	RequestSubsystem->BeginStreamRequest(RequestConfiguration);
	RequestSubsystem->EndStreamRequest();
}

/**
 * Called when a Wit entities request is successfully completed. The response will contain a list of available entities
 */
void FWitConfigurationUtilities::OnEntitiesRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	UE_LOG(LogWit, Verbose, TEXT("OnEntitiesRequestComplete - Final response size: %d"), BinaryResponse.Num());

	// We can't use the Json object because it will only contain the final entry of the array. We therefore use the binary data and convert it as an array
	// TODO - in Unity we actually grab the detailed entity data using /entities/<entity_name> 

	const FUTF8ToTCHAR DataAsCharacters(reinterpret_cast<const ANSICHAR*>(BinaryResponse.GetData()), BinaryResponse.Num());
	const FString JsonArrayString = FString(DataAsCharacters.Length(), DataAsCharacters.Get());

	const bool bIsConversionError = !FJsonObjectConverter::JsonArrayStringToUStruct<FWitEntityDefinition>( JsonArrayString, &Configuration->Application.Data.Entities);

	if (bIsConversionError)
	{
		OnEntitiesRequestError(TEXT("Json To UStruct failed"), TEXT("Converting the Json response to a UStruct failed"));
		return;
	}

	UE_LOG(LogWit, Verbose, TEXT("OnEntitiesRequestComplete - Received entities: %d"), Configuration->Application.Data.Entities.Num());

	RequestTraitList();
}

/**
 * Called when an entities request errors
 */
void FWitConfigurationUtilities::OnEntitiesRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage)
{
	UE_LOG(LogWit, Warning, TEXT("OnEntitiesRequestError - request failed with error: %s - %s"), *ErrorMessage, *HumanReadableErrorMessage);

	RequestTraitList();
}

/**
 * Requests the available list of traits for an app
 */
void FWitConfigurationUtilities::RequestTraitList()
{
	FWitRequestConfiguration RequestConfiguration{};

	if (!SetupListRequest(RequestConfiguration, EWitRequestEndpoint::GetTraits, true))
	{
		Configuration = nullptr;
		return;
	}

	RequestConfiguration.OnRequestError.AddStatic(&FWitConfigurationUtilities::OnTraitsRequestError);
	RequestConfiguration.OnRequestComplete.AddStatic(&FWitConfigurationUtilities::OnTraitsRequestComplete);

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	RequestSubsystem->BeginStreamRequest(RequestConfiguration);
	RequestSubsystem->EndStreamRequest();
}

/**
 * Called when a Wit traits request is successfully completed. The response will contain a list of available traits
 */
void FWitConfigurationUtilities::OnTraitsRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	UE_LOG(LogWit, Verbose, TEXT("OnTraitsRequestComplete - Final response size: %d"), BinaryResponse.Num());

	// We can't use the Json object because it will only contain the final entry of the array. We therefore use the binary data and convert it as an array
	// TODO - in Unity we actually grab the detailed traits data using /traits/<trait_name> 

	const FUTF8ToTCHAR DataAsCharacters(reinterpret_cast<const ANSICHAR*>(BinaryResponse.GetData()), BinaryResponse.Num());
	const FString JsonArrayString = FString(DataAsCharacters.Length(), DataAsCharacters.Get());

	const bool bIsConversionError = !FJsonObjectConverter::JsonArrayStringToUStruct<FWitTraitDefinition>( JsonArrayString, &Configuration->Application.Data.Traits);

	if (bIsConversionError)
	{
		OnTraitsRequestError(TEXT("Json To UStruct failed"), TEXT("Converting the Json response to a UStruct failed"));
		return;
	}

	UE_LOG(LogWit, Verbose, TEXT("OnTraitsRequestComplete - Received traits: %d"), Configuration->Application.Data.Traits.Num());

	RequestVoiceList();
}

/**
 * Called when an traits request errors
 */
void FWitConfigurationUtilities::OnTraitsRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage)
{
	UE_LOG(LogWit, Warning, TEXT("OnTraitsRequestError - request failed with error: %s - %s"), *ErrorMessage, *HumanReadableErrorMessage);

	RequestVoiceList();
}

/**
 * Requests the available list of voices for an app
 */
void FWitConfigurationUtilities::RequestVoiceList()
{
	FWitRequestConfiguration RequestConfiguration{};

	if (!SetupListRequest(RequestConfiguration, EWitRequestEndpoint::GetVoices, false))
	{
		Configuration = nullptr;
		return;
	}

	RequestConfiguration.OnRequestError.AddStatic(&FWitConfigurationUtilities::OnVoicesRequestError);
	RequestConfiguration.OnRequestComplete.AddStatic(&FWitConfigurationUtilities::OnVoicesRequestComplete);

	UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	RequestSubsystem->BeginStreamRequest(RequestConfiguration);
	RequestSubsystem->EndStreamRequest();
}

/**
 * Called when a Wit voices request is successfully completed. The response will contain a list of available voices
 */
void FWitConfigurationUtilities::OnVoicesRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse)
{
	UE_LOG(LogWit, Verbose, TEXT("OnVoicesRequestComplete - Final response size: %d"), BinaryResponse.Num());

	FWitVoicesResponse VoicesResponse;
	const bool bIsConversionError = !FJsonObjectConverter::JsonObjectToUStruct(JsonResponse.ToSharedRef(), &VoicesResponse);
	
	if (bIsConversionError)
	{
		OnVoicesRequestError(TEXT("Json To UStruct failed"), TEXT("Converting the Json response to a UStruct failed"));
		return;
	}

	Configuration->Application.Data.Voices.Append(VoicesResponse.En_US);

	UE_LOG(LogWit, Verbose, TEXT("OnVoicesRequestComplete - Received voices: %d"), Configuration->Application.Data.Voices.Num());

	(void)Configuration->MarkPackageDirty();
	Configuration = nullptr;
}

/**
 * Called when a voices request errors
 */
void FWitConfigurationUtilities::OnVoicesRequestError(const FString& ErrorMessage, const FString& HumanReadableErrorMessage)
{
	UE_LOG(LogWit, Warning, TEXT("OnVoicesRequestError - request failed with error: %s - %s"), *ErrorMessage, *HumanReadableErrorMessage);

	(void)Configuration->MarkPackageDirty();
	Configuration = nullptr;
}

/**
 * Common setup for a list request
 */
bool FWitConfigurationUtilities::SetupListRequest(FWitRequestConfiguration& RequestConfiguration, EWitRequestEndpoint Endpoint,
                                                  const bool bIsServerAuthRequired)
{
	if (Configuration == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("SetupListRequest: cannot fetch because no configuration found. Please assign a configuration"));
		return false;
	}

	FString AuthToken;

	if (bIsServerAuthRequired)
	{
		AuthToken = Configuration->Application.ServerAccessToken;
	}
	else
	{
		AuthToken = Configuration->Application.ClientAccessToken;
	}

	const bool bHasValidAuthToken = !AuthToken.IsEmpty();

	if (!bHasValidAuthToken)
	{
		UE_LOG(LogWit, Warning, TEXT("SetupListRequest: cannot fetch because no valid auth token. Please assign an auth token"));
		return false;
	}

	const UWitRequestSubsystem* RequestSubsystem = GEngine->GetEngineSubsystem<UWitRequestSubsystem>();

	if (RequestSubsystem == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("SetupListRequest: cannot fetch because request subsystem does not exist"));
		return false;
	}

	if (RequestSubsystem->IsRequestInProgress())
	{
		UE_LOG(LogWit, Warning, TEXT("SetupListRequest: cannot fetch because a request is already in progress"));
		return false;
	}

	FWitRequestBuilder::SetRequestConfigurationWithDefaults(RequestConfiguration, Endpoint, AuthToken, Configuration->Application.Advanced.ApiVersion,
	                                                        Configuration->Application.Advanced.URL);
	FWitRequestBuilder::AddFormatContentType(RequestConfiguration, EWitRequestFormat::Json);

	RequestConfiguration.bShouldUseCustomHttpTimeout = Configuration->Application.Advanced.bIsCustomHttpTimeout;
	RequestConfiguration.HttpTimeout = Configuration->Application.Advanced.HttpTimeout;

	return true;
}

/**
 * Is a refresh in progress
 */
bool FWitConfigurationUtilities::IsRefreshInProgress()
{
	return Configuration != nullptr;
}
