/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TTS/Cache/Storage/TtsStorageCache.h"
#include "TTS/Cache/Storage/Asset/TtsStorageCacheAsset.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Default constructor
 */
UTtsStorageCache::UTtsStorageCache()
{
	PrimaryComponentTick.bCanEverTick = false;
}

/**
 * Get the path to the given clip in the cache
 *
 * @param CacheLocation [in] the cache location where the clip will be 
 * @param CachePath [out] the full path to the clip in the cache
 *
 * @return false if the path does not exist or cannot be created
 */
bool UTtsStorageCache::GetCachePath(const ETtsStorageCacheLocation CacheLocation, FString& CachePath) const
{
	const ETtsStorageCacheLocation FinalCacheLocation = GetFinalCacheLocation(CacheLocation);
	
	switch (FinalCacheLocation)
	{
	case ETtsStorageCacheLocation::Content:
		CachePath = FPaths::ProjectContentDir();
		break;
	case ETtsStorageCacheLocation::Persistent:
		CachePath = FPlatformProcess::UserSettingsDir();
		break;
	case ETtsStorageCacheLocation::Temporary:
		CachePath = FPlatformProcess::UserTempDir();
		break;
	default:
		CachePath.Empty();
		return false;
	}
	
	CachePath.Append(CacheDirectory);

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	if (!FileManager.DirectoryExists(*CachePath))
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::GetCachePath: Cache directy does not exist so creating"));
		
		const bool bDidCreateDirectory = FileManager.CreateDirectoryTree(*CachePath);
		if (!bDidCreateDirectory)
		{
			UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::GetCachePath: Failed to create cache directory (%s)"), *CachePath);
			
			return false;
		}
	}

	if (!CachePath.EndsWith("/"))
	{
		CachePath.Append("/");
	}
	
	return true;
}

/**
 * Should we cache a clip?
 *
 * @param CacheLocation [in] the cache location where the clip will be 
 *
 * @return false if the path does not exist or cannot be created
 */
bool UTtsStorageCache::ShouldCache(const ETtsStorageCacheLocation CacheLocation) const
{
	return GetFinalCacheLocation(CacheLocation) != ETtsStorageCacheLocation::None;	
}

/**
 * Add a clip to the cache
 *
 * @param ClipId [in] the clip id
 * @param ClipData [in] the binary data that represents the clip
 * @param ClipSettings [in] the settings that were originally used to create the clip
 *
 * @return true if the clip was added
 */
bool UTtsStorageCache::AddClip(const FString& ClipId, const TArray<uint8>& ClipData, const FTtsConfiguration& ClipSettings)
{
	FString CacheFilePath;

	const bool bShouldCache = GetCachePath(ClipSettings.StorageCacheLocation, CacheFilePath);
	if (!bShouldCache)
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::AddClip: caching is disabled"));
		return false;
	}
	
	UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::AddClip: adding clip (%s) with path (%s) and data size (%d)"), *ClipId, *CacheFilePath, ClipData.Num());

	const bool bShouldSaveAsAsset = GetFinalCacheLocation(ClipSettings.StorageCacheLocation) == ETtsStorageCacheLocation::Content;
	if (bShouldSaveAsAsset)
	{
		// We can only save clips to the content folder when we are running in editor

#if WITH_EDITOR
		return SaveClipToAssetFile(ClipId, ClipData, ClipSettings);
#else
		return false;
#endif
	}

	CacheFilePath.Append(ClipId);
	
	return SaveClipToBinaryFile(CacheFilePath, ClipData);
}

/*
 * Save a clip in a UAsset file
 */
bool UTtsStorageCache::SaveClipToAssetFile(const FString& ClipId, const TArray<uint8>& ClipData, const FTtsConfiguration& ClipSettings) const
{
	const FString PackagePath = FString::Printf(TEXT("/Game/%s/%s"), *CacheDirectory, *ClipId);
	
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

	return true;
}

/*
 * Save a clip in a binary file
 */
bool UTtsStorageCache::SaveClipToBinaryFile(const FString& CacheFilePath, const TArray<uint8>& ClipData)
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

/**
 * Request a clip from the cache
 *
 * @param ClipId [in] the clip id
 * @param CacheLocation [in] the cache location where the clip will be 
 * @param ClipData [out] the binary data that represents the clip
 *
 * @return true if the clip is available in the cache
 */
bool UTtsStorageCache::RequestClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation, TArray<uint8>& ClipData) const
{
	FString CacheFilePath;
	
	const bool bShouldCache = GetCachePath(CacheLocation, CacheFilePath);
	if (!bShouldCache)
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RequestClip: caching is disabled"));
		return false;
	}
	
	UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RequestClip: requesting clip (%s) with path (%s)"), *ClipId, *CacheFilePath);

	const bool bShouldLoadFromAsset = GetFinalCacheLocation(CacheLocation) == ETtsStorageCacheLocation::Content;
	if (bShouldLoadFromAsset)
	{
		return LoadClipFromAssetFile(ClipId, ClipData);
	}
	
	CacheFilePath.Append(ClipId);
		
	return LoadClipFromBinaryFile(CacheFilePath, ClipData);
};

/*
 * Load a clip from a UAsset file
 */
bool UTtsStorageCache::LoadClipFromAssetFile(const FString& ClipId, TArray<uint8>& ClipData) const
{
	const FString PackagePath = FString::Printf(TEXT("/Game/%s/%s"), *CacheDirectory, *ClipId);
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
bool UTtsStorageCache::LoadClipFromBinaryFile(const FString& CacheFilePath, TArray<uint8>& ClipData)
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

/*
 * Get the final location to cache a clip taking into account overrides
 */
ETtsStorageCacheLocation UTtsStorageCache::GetFinalCacheLocation(const ETtsStorageCacheLocation CacheLocation) const
{
	if (CacheLocation == ETtsStorageCacheLocation::Default)
	{
		return DefaultCacheLocation;
	}

	return CacheLocation;
}

/**
 * Remove a clip from the cache
 *
 * @param ClipId [in] the clip id
 * @param CacheLocation [in] the cache location where the clip will be
 *
 * @return true if the clip was removed
 */	
bool UTtsStorageCache::RemoveClip(const FString& ClipId, const ETtsStorageCacheLocation CacheLocation)
{
	FString CacheFilePath;
	
	const bool bShouldCache = GetCachePath(CacheLocation, CacheFilePath);
	if (!bShouldCache)
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RemoveClip: caching is disabled"));
		return false;
	}
	
	const bool bIsAsset = GetFinalCacheLocation(CacheLocation) == ETtsStorageCacheLocation::Content;
	if (bIsAsset)
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RemoveClip: cannot delete assets from the content directory"));
		return false;
	}
	
	CacheFilePath.Append(ClipId);

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
				
	if (!FileManager.FileExists(*CacheFilePath))
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RemoveClip: clip does exist in cache (%s)"), *CacheFilePath);
		return false;
	}
	
	const bool bDidDeleteSuccessfully = FileManager.DeleteFile(*CacheFilePath);
	if (!bDidDeleteSuccessfully)
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RemoveClip: failed to delete cached file (%s)"), *CacheFilePath);
		return false;
	}

	UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RemoveClip: removing clip (%s) with path (%s)"), *ClipId, *CacheFilePath);

	return true;
}

/**
 * Remove all clips from the cache
 *
 * @param CacheLocation [in] the cache location where the clip will be
 */	
void UTtsStorageCache::RemoveAllClips(const ETtsStorageCacheLocation CacheLocation)
{
	FString CacheFilePath;
	
	const bool bShouldCache = GetCachePath(CacheLocation, CacheFilePath);
	if (!bShouldCache)
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RemoveAllClips: caching is disabled"));
		return;
	}

	const bool bIsAsset = GetFinalCacheLocation(CacheLocation) == ETtsStorageCacheLocation::Content;
	if (bIsAsset)
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RemoveAllClips: cannot delete assets from the content directory"));
		return;
	}
	
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
				
	if (!FileManager.DirectoryExists(*CacheFilePath))
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RemoveAllClips: cache directory does not exist"));
		return;
	}

	const bool bDidDeleteSuccessfully = FileManager.DeleteDirectoryRecursively(*CacheFilePath);
	if (!bDidDeleteSuccessfully)
	{
		UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RemoveAllClips: failed to delete cache directory"));
		return;
	}

	UE_LOG(LogWit, Verbose, TEXT("UTTSStorageCache::RemoveAllClips: removing all clips with path (%s)"), *CacheFilePath);
}
