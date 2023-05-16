/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Utilities/WitHelperUtilities.h"
#include "JsonObjectConverter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/BufferArchive.h"
#include "TTS/Cache/Storage/Asset/TtsStorageCacheAsset.h"
#include "Wit/Utilities/WitLog.h"
#include "Misc/EngineVersionComparison.h"
#include "HAL/PlatformFileManager.h"
#include "UObject/SavePackage.h"

FString FWitHelperUtilities::AdditionalFrontUserData = "";
FString FWitHelperUtilities::AdditionalEndUserData = "";

 /**
  * Adds a string to the user agent data to include in Wit web requests
  */
void FWitHelperUtilities::AddRequestUserData(const FString& UserData, const bool AddToFront)
{
	if (AddToFront)
	{
		AdditionalFrontUserData = AdditionalFrontUserData == ""
			? FString::Printf(TEXT("%s,"), *UserData)
			: FString::Printf(TEXT("%s%s,"), *AdditionalFrontUserData, *UserData);
	}
	else
	{
		AdditionalEndUserData = AdditionalEndUserData == ""
			? FString::Printf(TEXT(",%s"), *UserData)
			: FString::Printf(TEXT("%s,%s"), *AdditionalEndUserData, *UserData);
	}
}

/**
 * Finds the VoiceExperience in the scene. This is slow so do not call every frame
 * 
 * @return pointer to the Voice Experience actor if found otherwise null
 */
AVoiceExperience* FWitHelperUtilities::FindVoiceExperience(const UWorld* World, const FName& Tag)
{
	check(World != nullptr);

	AVoiceExperience* VoiceExperience = nullptr;
	const bool bIsValidTag = !Tag.IsNone() && Tag.IsValid();

	if (bIsValidTag)
	{
		UE_LOG(LogWit, Verbose, TEXT("FindVoiceExperience: Trying to find Voice Experience with tag %s"), *Tag.ToString());
		
		TArray<AActor*> VoiceExperiences;
		
		UGameplayStatics::GetAllActorsOfClassWithTag(World, AVoiceExperience::StaticClass(), Tag, VoiceExperiences);

		// If more than 1 match then just use the first

		const bool bIsVoiceExperienceWithMatchingTag = VoiceExperiences.Num() > 0;
		 
		if (bIsVoiceExperienceWithMatchingTag)
		{
			UE_LOG(LogWit, Verbose, TEXT("FindVoiceExperience: Found Voice Experience with tag %s"), *Tag.ToString());
			
			VoiceExperience = static_cast<AVoiceExperience*>(VoiceExperiences[0]);
		}
	}

	// If we don't find a voice experience with a matching tag then we try to find the first voice experience
	
	if (VoiceExperience == nullptr)
	{
		VoiceExperience = static_cast<AVoiceExperience*>(UGameplayStatics::GetActorOfClass(World, AVoiceExperience::StaticClass()));
	}

	// If we still find nothing then not much we can do
	
	if (VoiceExperience == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("FindVoiceExperience: No Voice Experience actor found"));
		return nullptr;
	}

	return VoiceExperience;
}

/**
 * Finds the TtsExperience in the scene. This is slow so do not call every frame
 * 
 * @return pointer to the TTS Experience actor if found otherwise null
 */
ATtsExperience* FWitHelperUtilities::FindTtsExperience(const UWorld* World, const FName& Tag)
{
	check(World != nullptr);

	ATtsExperience* TtsExperience = nullptr;
	const bool bIsValidTag = !Tag.IsNone() && Tag.IsValid();

	if (bIsValidTag)
	{
		UE_LOG(LogWit, Verbose, TEXT("FindTtsExperience: Trying to find TTS Experience with tag %s"), *Tag.ToString());
		
		TArray<AActor*> TtsExperiences;
		
		UGameplayStatics::GetAllActorsOfClassWithTag(World, ATtsExperience::StaticClass(), Tag, TtsExperiences);

		// If more than 1 match then just use the first

		const bool bIsVoiceExperienceWithMatchingTag = TtsExperiences.Num() > 0;
		 
		if (bIsVoiceExperienceWithMatchingTag)
		{
			UE_LOG(LogWit, Verbose, TEXT("FindTtsExperience: Found TTS Experience with tag %s"), *Tag.ToString());
			
			TtsExperience = static_cast<ATtsExperience*>(TtsExperiences[0]);
		}
	}

	// If we don't find a TTS experience with a matching tag then we try to find the first TTS experience
	
	if (TtsExperience == nullptr)
	{
		TtsExperience = static_cast<ATtsExperience*>(UGameplayStatics::GetActorOfClass(World, ATtsExperience::StaticClass()));
	}

	// If we still find nothing then not much we can do
	
	if (TtsExperience == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("FindTtsExperience: No TTS Experience actor found"));
		return nullptr;
	}

	return TtsExperience;
}

/**
 * Finds the DictationExperience in the scene. This is slow so do not call every frame
 * 
 * @return pointer to the Dictation Experience actor if found otherwise null
 */
ADictationExperience* FWitHelperUtilities::FindDictationExperience(const UWorld* World, const FName& Tag)
{
	check(World != nullptr);

	ADictationExperience* DictationExperience = nullptr;
	const bool bIsValidTag = !Tag.IsNone() && Tag.IsValid();

	if (bIsValidTag)
	{
		UE_LOG(LogWit, Verbose, TEXT("FindDictationExperience: Trying to find Dictation Experience with tag %s"), *Tag.ToString());
		
		TArray<AActor*> DictationExperiences;
		
		UGameplayStatics::GetAllActorsOfClassWithTag(World, ADictationExperience::StaticClass(), Tag, DictationExperiences);

		// If more than 1 match then just use the first

		const bool bIsVoiceExperienceWithMatchingTag = DictationExperiences.Num() > 0;
		 
		if (bIsVoiceExperienceWithMatchingTag)
		{
			UE_LOG(LogWit, Verbose, TEXT("FindDictationExperience: Found Dictation Experience with tag %s"), *Tag.ToString());
			
			DictationExperience = static_cast<ADictationExperience*>(DictationExperiences[0]);
		}
	}

	// If we don't find a dictation experience with a matching tag then we try to find the first TTS experience
	
	if (DictationExperience == nullptr)
	{
		DictationExperience = static_cast<ADictationExperience*>(UGameplayStatics::GetActorOfClass(World, ADictationExperience::StaticClass()));
	}

	// If we still find nothing then not much we can do
	
	if (DictationExperience == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("FindDictationExperience: No Dictation Experience actor found"));
		return nullptr;
	}

	return DictationExperience;
}

/**
 * Tries to find an entity in the response with the given name
 * 
 * @param Response [in] the response to check
 * @param EntityName [in] the entity name to look for
 * @param ConfidenceThreshold [in] the threshold to exceed for it to be considered a match
 * @return pointer to the matching entity if found otherwise null
 */
const FWitEntity* FWitHelperUtilities::FindMatchingEntity(const FWitResponse& Response, const FString& EntityName, const float ConfidenceThreshold)
{
	const bool bIsNoEntity = Response.Entities.Num() == 0;
	
	if (bIsNoEntity)
	{
		return nullptr;
	}

	const FWitEntity* MatchingEntity = Response.Entities.Find(EntityName);

	if (MatchingEntity == nullptr)
	{
		return nullptr;
	}

	if (MatchingEntity->Confidence <= ConfidenceThreshold)
	{
		return nullptr;
	}

	return MatchingEntity;
}

	
/** 
 * Tries to find entities in the response with the given name
 * 
 * @param Response [in] the response to check
 * @param EntityName [in] the entity name to look for
 * @param ConfidenceThreshold [in] the threshold to exceed for it to be considered a match
 * @param MatchingEntities [out] matching entities
 * @return true if found any, otherwise false.
 */
bool FWitHelperUtilities::FindMatchingEntities(const FWitResponse& Response, const FString& EntityName, const float ConfidenceThreshold, FWitEntities& MatchingEntities)
{
	const bool bIsNoEntity = Response.AllEntities.Num() == 0;
	
	if (bIsNoEntity)
	{
		return false;
	}

	const FWitEntities* Entities = Response.AllEntities.Find(EntityName);

	if (Entities == nullptr)
	{
		return false;
	}
	
	for (auto& MatchingEntity : Entities->Entities)
	{
		if (MatchingEntity.Confidence > ConfidenceThreshold)
		{
			MatchingEntities.Entities.Add(MatchingEntity);
		}
	}

	if (MatchingEntities.Entities.Num() ==0)
	{
		return false;
	}
	return true;
}

/**
 * Tries to find an intent in the response with the given name
 * 
 * @param Response [in] the response to check
 * @param IntentName [in] the intent name to look for
 * @param ConfidenceThreshold [in] the threshold to exceed for it to be considered a match
 * @return pointer to the matching intent if found otherwise null
 */
const FWitIntent* FWitHelperUtilities::FindMatchingIntent(const FWitResponse& Response, const FString& IntentName, const float ConfidenceThreshold)
{
	const bool bIsNoIntent = (Response.Intents.Num() == 0);
	
	if (bIsNoIntent)
	{
		return nullptr;
	}

	const FWitIntent& FirstIntent{Response.Intents[0]};
	const bool bIsMatchingIntent = FirstIntent.Name.Equals(IntentName) && FirstIntent.Confidence > ConfidenceThreshold;
	
	if (!bIsMatchingIntent)
	{
		return nullptr;
	}

	return &FirstIntent;
}

/**
 * Calculates a unique Id for a text and voice configuration
 * 
 * @param ClipSettings [in] the settings used for the conversion
 * @return the unique id
 */
FString FWitHelperUtilities::GetVoiceClipId(const FTtsConfiguration& ClipSettings)
{
	// Serialize the text and configuration to a byte buffer and then generate a hash from that to use as the ID
		
	FBufferArchive Buffer;

	// We don't want to include the storage location in the hash otherwise it will change the hash value
	
	FTtsConfiguration HashSettings(ClipSettings);
	HashSettings.StorageCacheLocation = ETtsStorageCacheLocation::Default;
	
	FTtsConfiguration::StaticStruct()->SerializeBin(Buffer, &HashSettings);

	FSHAHash OutputHash;
	FSHA1::HashBuffer(Buffer.GetData(), Buffer.Num(), OutputHash.Hash);

	return OutputHash.ToString();
}

/**
 * Creates a sound wave from raw data
 * 
 * @param RawData [in] the raw binary data
 * @param RawDataSize [in] the raw data size
 * @return the sound wave created or null if unsuccessful
 */
USoundWave* FWitHelperUtilities::CreateSoundWaveFromRawData(const uint8* RawData, const int32 RawDataSize)
{
	FWaveModInfo WaveInfo;
	const bool bIsValidWaveInfo = WaveInfo.ReadWaveInfo(RawData, RawDataSize);	

	if (!bIsValidWaveInfo)
	{
		return nullptr;
	}

	const int32 ChannelCount = *WaveInfo.pChannels;
	const bool bIsMonoOrStereo = ChannelCount > 0 && ChannelCount <= 2;

	if (!bIsMonoOrStereo)
	{
		return nullptr;
	}
	
	const int32 SizeOfSample = (*WaveInfo.pBitsPerSample) / 8;
	const int32 NumSamples = WaveInfo.SampleDataSize / SizeOfSample;
	const int32 NumFrames = NumSamples / ChannelCount;
	const bool bIsEmpty = NumFrames <= 0;

	if (bIsEmpty)
	{
		return nullptr;
	}
	
	const float Duration = static_cast<float>(NumFrames) / *WaveInfo.pSamplesPerSec;
	
	UE_LOG(LogWit, Verbose, TEXT("Wave Info: Channel count (%d) duration (%f) sample data size (%d) sample rate (%d) bits per sample (%d) raw size (%d)"),
		*WaveInfo.pChannels, Duration, WaveInfo.SampleDataSize, *WaveInfo.pSamplesPerSec, *WaveInfo.pBitsPerSample, RawDataSize);

	USoundWave* SoundWave = NewObject<USoundWave>(USoundWave::StaticClass()); 

	SoundWave->Duration = Duration;
	SoundWave->SetSampleRate(*WaveInfo.pSamplesPerSec);
	SoundWave->NumChannels = *WaveInfo.pChannels;
	SoundWave->TotalSamples = *WaveInfo.pSamplesPerSec * SoundWave->Duration;
	SoundWave->SoundGroup = SOUNDGROUP_Default;

	// This is the preview data for in-editor

#if UE_VERSION_OLDER_THAN(5,1,0)
	SoundWave->RawData.Lock(LOCK_READ_WRITE);
	void* LockedData = SoundWave->RawData.Realloc(RawDataSize);
	FMemory::Memcpy(LockedData, RawData, RawDataSize);
	SoundWave->RawData.Unlock();
#elif WITH_EDITORONLY_DATA
	const FSharedBuffer SharedBuffer = FSharedBuffer::Clone(RawData, RawDataSize);
	SoundWave->RawData.UpdatePayload(SharedBuffer);	
#endif
	
	// This is the PCM data for packaged builds
	
	SoundWave->RawPCMDataSize = WaveInfo.SampleDataSize;
	SoundWave->RawPCMData = static_cast<uint8*>(FMemory::Malloc(SoundWave->RawPCMDataSize));
	FMemory::Memmove(SoundWave->RawPCMData, WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);

	return SoundWave;
}

/*
 * Save a clip in a UAsset file
 */
bool FWitHelperUtilities::SaveClipToAssetFile(const FString& ClipDirectory, const FString& ClipId, const TArray<uint8>& ClipData, const FTtsConfiguration& ClipSettings)
{
	FString PackagePath;

	if (ClipDirectory.IsEmpty())
	{
		PackagePath = FString::Printf(TEXT("/Game/%s"), *ClipId);
	}
	else
	{
		PackagePath = FString::Printf(TEXT("/Game/%s/%s"), *ClipDirectory, *ClipId);
	}
	
	UPackage* CachePackage = CreatePackage(*PackagePath);

	if (CachePackage == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("UTTSStorageCache::SaveClipToAssetFile: failed to create package file for (%s)"), *ClipId);
		return false;
	}
	
	UTtsStorageCacheAsset* CacheAsset = NewObject<UTtsStorageCacheAsset>(CachePackage, UTtsStorageCacheAsset::StaticClass(), *ClipId, RF_Public | RF_Standalone);

	if (CacheAsset == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("UTTSStorageCache::SaveClipToAssetFile: failed to create asset file for (%s)"), *ClipId);
		return false;
	}

	CacheAsset->ClipSettings = ClipSettings;
	
	CacheAsset->ClipData.AddUninitialized(ClipData.Num());
	FMemory::Memcpy(CacheAsset->ClipData.GetData(), ClipData.GetData(), ClipData.Num());
			
	(void)CacheAsset->MarkPackageDirty();
	CachePackage->MarkPackageDirty();
	
	FAssetRegistryModule::AssetCreated(CacheAsset);
	const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());

	SaveAssetFile(CachePackage, CacheAsset, PackageFileName);
	
	return true;
}

/*
 * Save a UAsset file
 */
void FWitHelperUtilities::SaveAssetFile(UPackage* Package, UDataAsset* Asset, const FString FileName)
{
#if UE_VERSION_OLDER_THAN(5,0,0)
	UPackage::SavePackage(Package, Asset, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FileName, GError, nullptr, true, true, SAVE_NoError);
#elif UE_VERSION_OLDER_THAN(5,1,0)
	const FSavePackageArgs SaveArgs = { nullptr, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, SAVE_NoError, true, true, true, FDateTime::Now(), GError };
	UPackage::SavePackage(Package, Asset, *FileName, SaveArgs);
#else
	const FSavePackageArgs SaveArgs = { nullptr, nullptr, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, SAVE_NoError, true, true, true, FDateTime::Now(), GError };
	UPackage::SavePackage(Package, Asset, *FileName, SaveArgs);
#endif
}

/*
 * Save a clip in a binary file
 */
bool FWitHelperUtilities::SaveClipToBinaryFile(const FString& CacheFilePath, const TArray<uint8>& ClipData)
{
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	
	if (FileManager.FileExists(*CacheFilePath))
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::SaveClipToBinaryFile: file already exists so no need to add to cache"));
		return false;
	}

	const TUniquePtr<IFileHandle> FileHandle(FileManager.OpenWrite(*CacheFilePath));
		
	if (FileHandle == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("UTTSStorageCache::SaveClipToBinaryFile: failed to open file for writing (%s)"), *CacheFilePath);
		return false;;
	}

	const bool bDidWriteSuccessfully = FileHandle->Write(ClipData.GetData(), ClipData.Num());
	if (!bDidWriteSuccessfully)
	{
		UE_LOG(LogWit, Warning, TEXT("UTTSStorageCache::SaveClipToBinaryFile: failed to write contents to file (%s)"), *CacheFilePath);
		return false;
	}
	
	return true;
}

/*
 * Load a clip from a UAsset file
 */
bool FWitHelperUtilities::LoadClipFromAssetFile(const FString& ClipDirectory, const FString& ClipId, TArray<uint8>& ClipData)
{
	FString PackagePath;

	if (ClipDirectory.IsEmpty())
	{
		PackagePath = FString::Printf(TEXT("/Game/%s"), *ClipId);
	}
	else
	{
		PackagePath = FString::Printf(TEXT("/Game/%s/%s"), *ClipDirectory, *ClipId);
	}
	
	UTtsStorageCacheAsset* CacheAsset = LoadObject<UTtsStorageCacheAsset>(nullptr, *PackagePath);

	if (CacheAsset == nullptr)
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::LoadClipFromAssetFile: clip does exist in cache or cannot be loaded (%s)"), *PackagePath);
		return false;
	}

	ClipData.AddUninitialized(CacheAsset->ClipData.Num());
	FMemory::Memcpy(ClipData.GetData(), CacheAsset->ClipData.GetData(), CacheAsset->ClipData.Num());
		
	UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::LoadClipFromAssetFile: read clip data of size (%d)"), ClipData.Num());

	return true;
}

/*
 * Load a clip from a binary file
 */
bool FWitHelperUtilities::LoadClipFromBinaryFile(const FString& CacheFilePath, TArray<uint8>& ClipData)
{
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
				
	if (!FileManager.FileExists(*CacheFilePath))
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::LoadClipFromBinaryFile: clip does exist in cache"));
		return false;
	}

	const TUniquePtr<IFileHandle> FileHandle(FileManager.OpenRead(*CacheFilePath));
			
	if (FileHandle == nullptr)
	{
		UE_LOG(LogWit, Warning, TEXT("UTTSStorageCache::LoadClipFromBinaryFile: failed to open file for reading (%s)"), *CacheFilePath);
		return false;
	}
			
	ClipData.AddUninitialized(FileHandle->Size());
	const bool bDidReadSuccessfully = FileHandle->Read(ClipData.GetData(),ClipData.Num());
	
	if (!bDidReadSuccessfully)
	{
		UE_LOG(LogWit, Warning, TEXT("UTTSStorageCache::LoadClipFromBinaryFile: failed to read contents of file (%s)"), *CacheFilePath);		
		return false;
	}

	UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::LoadClipFromBinaryFile: read clip data of size (%d)"), ClipData.Num());

	return true;
}

bool FWitHelperUtilities::IsWitResponse(const TSharedPtr<FJsonObject> JsonResponse)
{
	const TArray< TSharedPtr<FJsonValue> >* OutArray;
	return JsonResponse->TryGetArrayField("intents", OutArray);
}

void FWitHelperUtilities::ConvertJsonToAllEntities(FWitResponse* WitResponse, const TSharedPtr<FJsonObject>* EntitiesJsonObject)
{
	for (const auto& Entities : WitResponse->Entities)
	{
		const FString Key = *Entities.Key;

		FWitEntities* WitEntities = WitResponse->AllEntities.Find(Key);
		if (WitEntities == nullptr)
		{
			WitResponse->AllEntities.Add(Key, FWitEntities());
			WitEntities = WitResponse->AllEntities.Find(Key);
		}
		
		const TArray< TSharedPtr<FJsonValue> >* GroupOfEntitiesJsonObject;
		const bool bIsKeyExist = EntitiesJsonObject->Get()->TryGetArrayField(Key, GroupOfEntitiesJsonObject);
		UE_LOG(LogWit, Verbose, TEXT("Does key(%s) exist: %s"), *Key, bIsKeyExist?TEXT("YES"):TEXT("NO"));
		if (!bIsKeyExist)
		{
			continue;
		}

		for (auto& EntityJsonValue : *GroupOfEntitiesJsonObject)
		{
			const TSharedPtr<FJsonObject> EntityJsonObject = EntityJsonValue->AsObject();
			FWitEntity WitEntity{};
			const bool bIsEntityConversionError = !FJsonObjectConverter::JsonObjectToUStruct(EntityJsonObject.ToSharedRef(), &WitEntity);
			if (bIsEntityConversionError)
			{
				continue;
			}
			WitEntities->Name = Key;
			WitEntities->Entities.Add(WitEntity);
		}
	}
}

bool FWitHelperUtilities::ConvertJsonToWitResponse(const TSharedPtr<FJsonObject> JsonResponse, FWitResponse* WitResponse)
{
	const TSharedPtr<FJsonObject>* AllEntitiesJsonObject;
	JsonResponse->TryGetObjectField("entities", AllEntitiesJsonObject);
	
	const bool bIsConversionError = !FJsonObjectConverter::JsonObjectToUStruct(JsonResponse.ToSharedRef(), WitResponse);
	if (bIsConversionError)
	{
		return false;
	}

	for (const auto& Intent : WitResponse->Intents)
	{
		UE_LOG(LogWit, Verbose, TEXT("UStruct - Intent: id (%lld) name (%s) confidence (%f)"), Intent.Id, *Intent.Name, Intent.Confidence);
	}
	
	for (const auto& Entity : WitResponse->Entities)
	{
		UE_LOG(LogWit, Verbose, TEXT("UStruct - Entity (%s): id (%lld) name (%s) value (%s) confidence (%f) unit (%s) start (%d) end (%d)"), *Entity.Key,
			   Entity.Value.Id, *Entity.Value.Name, *Entity.Value.Value, Entity.Value.Confidence, *Entity.Value.Unit, Entity.Value.Start, Entity.Value.End);

		UE_LOG(LogWit, Verbose, TEXT("UStruct - Entity (%s): normalized value (%s) normalized unit (%s)"), *Entity.Key, *Entity.Value.Normalized.Value,
			   *Entity.Value.Normalized.Unit);
	}
	
	ConvertJsonToAllEntities(WitResponse, AllEntitiesJsonObject);

	if (WitResponse->AllEntities.Num() > 0)
	{
		UE_LOG(LogWit, Verbose, TEXT("All Entities: "));
	}
	for (const auto& Entities : WitResponse->AllEntities)
	{
		for (const auto& Entity :Entities.Value.Entities)
		{
			UE_LOG(LogWit, Verbose, TEXT("UStruct - Entity (%s): id (%lld) name (%s) value (%s) confidence (%f) unit (%s) start (%d) end (%d)"), *Entities.Key,
				   Entity.Id, *Entity.Name, *Entity.Value, Entity.Confidence, *Entity.Unit, Entity.Start, Entity.End);

			UE_LOG(LogWit, Verbose, TEXT("UStruct - Entity (%s): normalized value (%s) normalized unit (%s)"), *Entities.Key, *Entity.Normalized.Value,
				   *Entity.Normalized.Unit);
		}
	}

	for (const auto& Trait : WitResponse->Traits)
	{
		UE_LOG(LogWit, Verbose, TEXT("UStruct - Trait (%s): id (%lld) value (%s) confidence (%f)"), *Trait.Key, Trait.Value.Id, *Trait.Value.Value,
			   Trait.Value.Confidence);
	}
	
	return true;
}

void FWitHelperUtilities::AcceptPartialResponseAndCancelRequest(const UWorld* World, const FName& Tag, const FWitResponse& Response)
{
	AVoiceExperience* VoiceExperience = FWitHelperUtilities::FindVoiceExperience(World, Tag);
	
	if (VoiceExperience == nullptr)
	{
		return;
	}
	
	VoiceExperience->AcceptPartialResponseAndCancelRequest(Response);
}
