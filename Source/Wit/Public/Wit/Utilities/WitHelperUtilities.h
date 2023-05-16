/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Dictation/Experience/DictationExperience.h"
#include "TTS/Configuration/TtsConfiguration.h"
#include "TTS/Experience/TtsExperience.h"
#include "Voice/Experience/VoiceExperience.h"

struct FWitEntity;
struct FWitIntent;
struct FWitResponse;
class UWitVoiceService;
class AWitVoiceExperience;
class FJsonObject;
class FWitHttpRequest;

/**
 * A helper class that contains useful API utilities
 */
class WIT_API FWitHelperUtilities
{
public:

	friend class FWitHttpRequest;

	/**
	 * Adds a string to the user agent data to include in Wit web requests
	 */
	static void AddRequestUserData(const FString& UserData, const bool AddToFront = false);

	/**
	 * Finds the VoiceExperience in the scene. This is slow so do not call every frame
	 * 
	 * @return pointer to the Voice Experience actor if found otherwise null
	 */
	static AVoiceExperience* FindVoiceExperience(const UWorld* World, const FName& Tag);

	/**
	 * Finds the TtsExperience in the scene. This is slow so do not call every frame
	 * 
	 * @return pointer to the TTS Experience actor if found otherwise null
	 */
	static ATtsExperience* FindTtsExperience(const UWorld* World, const FName& Tag);

	/**
	 * Finds the DictationExperience in the scene. This is slow so do not call every frame
	 * 
	 * @return pointer to the Dictation Experience actor if found otherwise null
	 */
	static ADictationExperience* FindDictationExperience(const UWorld* World, const FName& Tag);
	
	/** 
	 * Tries to find an entity in the response with the given name
	 * 
	 * @param Response [in] the response to check
	 * @param EntityName [in] the entity name to look for
	 * @param ConfidenceThreshold [in] the threshold to exceed for it to be considered a match
	 * @return pointer to the matching entity if found otherwise null
	 */
	static const FWitEntity* FindMatchingEntity(const FWitResponse& Response, const FString& EntityName, const float ConfidenceThreshold);
	
	/** 
	 * Tries to find entities in the response with the given name
	 * 
	 * @param Response [in] the response to check
	 * @param EntityName [in] the entity name to look for
	 * @param ConfidenceThreshold [in] the threshold to exceed for it to be considered a match
	 * @param MatchingEntities [out] matching entities
 	 * @return true if found any, otherwise false.
	 */
	static bool FindMatchingEntities(const FWitResponse& Response, const FString& EntityName, const float ConfidenceThreshold, FWitEntities& MatchingEntities);

	/**
	 * Tries to find an intent in the response with the given name
	 * 
	 * @param Response [in] the response to check
	 * @param IntentName [in] the intent name to look for
	 * @param ConfidenceThreshold [in] the threshold to exceed for it to be considered a match
	 * @return pointer to the matching intent if found otherwise null
	 */
	static const FWitIntent* FindMatchingIntent(const FWitResponse& Response, const FString& IntentName, const float ConfidenceThreshold);

	/**
	 * Calculates a unique Id for a text and voice configuration
	 * 
	 * @param ClipSettings [in] the settings used for the conversion
	 * @return the unique id
	 */
	static FString GetVoiceClipId(const FTtsConfiguration& ClipSettings);

	/**
	 * Creates a sound wave from raw data
	 * 
	 * @param RawData [in] the raw binary data
	 * @param RawDataSize [in] the raw data size
	 * @return the sound wave created or null if unsuccessful
	 */
	static USoundWave* CreateSoundWaveFromRawData(const uint8* RawData, const int32 RawDataSize);
	
	/* Load a clip stored in a UAsset file */
	static bool LoadClipFromAssetFile(const FString& ClipDirectory, const FString& ClipId, TArray<uint8>& ClipData);

	/* Load a clip stored in a binary file */
	static bool LoadClipFromBinaryFile(const FString& CacheFilePath, TArray<uint8>& ClipData);

	/* Save a clip in a UAsset file */
	static bool SaveClipToAssetFile(const FString& ClipDirectory, const FString& ClipId, const TArray<uint8>& ClipData, const FTtsConfiguration& ClipSettings);

	/* Save a UAsset file */
	static void SaveAssetFile(UPackage* Package, UDataAsset* Asset, const FString FileName);

	/* Save a clip in a binary file */
	static bool SaveClipToBinaryFile(const FString& CacheFilePath, const TArray<uint8>& ClipData);

	/**
	 * Check whether the JsonResponse is Wit Response or not
	 * 
	 * @param JsonResponse [in] the Json Response
	 * @return Whether the JsonResponse is final or not
	 */
	static bool IsWitResponse(const TSharedPtr<FJsonObject> JsonResponse);

	/**
	 * Convert Json object to FWitResponse
	 * 
	 * @param JsonResponse [in] the Json Response
	 * @param WitResponse [in] the FWitResponse used to return.
	 * @return Whether the converting is done successfully
	 */
	static bool ConvertJsonToWitResponse (const TSharedPtr<FJsonObject> JsonResponse, FWitResponse* WitResponse);

	/**
	 * Convert Json object to WitEntities
	 * 
	 * @param WitResponse [in] the FWitResponse used to return.
	 * @param EntitiesJsonObject [in] the Json Response
	 * @return Whether the converting is done successfully
	 */
	static void ConvertJsonToAllEntities(FWitResponse* WitResponse, const TSharedPtr<FJsonObject>* EntitiesJsonObject);

	/**
	 * Accept the given Partial Response and cancel the current request.
	 * 
	 * This is for Live Understanding.
	 * Caller can use this method to early finish the request, if they found a partial response matches their expectation.
	 * 
	 * @param World [in] the game world
	 * @param Tag [in] the tag for VoiceExperience, used to find out the right Experience to cancel the request.
	 * @param Response [in] the Parity Response to accept. It later will be used for final response.
	 */
	static void AcceptPartialResponseAndCancelRequest(const UWorld* World, const FName& Tag, const FWitResponse& Response);

private:

	/** Additional user data to add to the front of user agent data in Wit requests */
	static FString AdditionalFrontUserData;

	/** Additional user data to add to the end of user agent data in Wit requests */
	static FString AdditionalEndUserData;
};
