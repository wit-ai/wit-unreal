/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Utilities/WitHelperUtilities.h"

#include "JsonObjectConverter.h"
#include "Wit/Voice/WitVoiceExperience.h"
#include "Wit/Voice/WitVoiceService.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/BufferArchive.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Finds the WitVoiceExperience in the scene. This is slow so do not call every frame
 * 
 * @return pointer to the WitAPI component if found otherwise null
 */
AVoiceExperience* FWitHelperUtilities::FindVoiceExperience(const UWorld* World, const FName& Tag)
{
	check(World != nullptr);

	AVoiceExperience* VoiceExperience = nullptr;
	const bool bIsValidTag = !Tag.IsNone() && Tag.IsValid();

	if (bIsValidTag)
	{
		UE_LOG(LogWit, Verbose, TEXT("FindWitVoiceExperience: Trying to find Voice Experience with tag %s"), *Tag.ToString());
		
		TArray<AActor*> VoiceExperiences;
		
		UGameplayStatics::GetAllActorsOfClassWithTag(World, AVoiceExperience::StaticClass(), Tag, VoiceExperiences);

		// If more than 1 match then just use the first

		const bool bIsVoiceExperienceWithMatchingTag = VoiceExperiences.Num() > 0;
		 
		if (bIsVoiceExperienceWithMatchingTag)
		{
			UE_LOG(LogWit, Verbose, TEXT("FindWitVoiceExperience: Found Voice Experience with tag %s"), *Tag.ToString());
			
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
		UE_LOG(LogWit, Warning, TEXT("FindWitVoiceExperience: No Voice Experience actor found"));
		return nullptr;
	}

	return VoiceExperience;
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
	
	SoundWave->RawData.Lock(LOCK_READ_WRITE);
	void* LockedData = SoundWave->RawData.Realloc(RawDataSize);
	FMemory::Memcpy(LockedData, RawData, RawDataSize);
	SoundWave->RawData.Unlock();

	// This is the PCM data for packaged builds
	
	SoundWave->RawPCMDataSize = WaveInfo.SampleDataSize;
	SoundWave->RawPCMData = static_cast<uint8*>(FMemory::Malloc(SoundWave->RawPCMDataSize));
	FMemory::Memmove(SoundWave->RawPCMData, WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);

	return SoundWave;
}

bool FWitHelperUtilities::IsWitResponse(const TSharedPtr<FJsonObject> JsonResponse)
{
	const TArray< TSharedPtr<FJsonValue> >* OutArray;
	return JsonResponse->TryGetArrayField("intents", OutArray);
}

bool FWitHelperUtilities::ConvertJsonToWitResponse(const TSharedPtr<FJsonObject> JsonResponse, FWitResponse* WitResponse)
{
	
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

	for (const auto& Trait : WitResponse->Traits)
	{
		UE_LOG(LogWit, Verbose, TEXT("UStruct - Trait (%s): id (%lld) value (%s) confidence (%f)"), *Trait.Key, Trait.Value.Id, *Trait.Value.Value,
			   Trait.Value.Confidence);
	}
	return true;
	
}

void FWitHelperUtilities::AcceptPartialResponseAndCancelRequest(const UWorld* World, const FName& Tag, const FWitResponse& Response)
{
	
	const AVoiceExperience* VoiceExperience = FWitHelperUtilities::FindVoiceExperience(World, Tag);
	
	if (VoiceExperience == nullptr)
	{
		return;
	}
	VoiceExperience->AcceptPartialResponseAndCancelRequest(Response);
}
