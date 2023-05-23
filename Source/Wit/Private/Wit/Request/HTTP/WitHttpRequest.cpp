/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if WITH_CURL

#include "WitHttpRequest.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "WitModule.h"
#include "Wit/Utilities/WitHelperUtilities.h"
#include "Wit/Utilities/WitLog.h"

/**
 * Destructor
 */
FWitHttpRequest::~FWitHttpRequest()
{
	if (StreamHeaderList)
	{
		curl_slist_free_all(StreamHeaderList);
		StreamHeaderList = nullptr;
	}
}

/**
 * Set the request payload from a stream
 */
bool FWitHttpRequest::SetContentFromStream(TSharedRef<FArchive, ESPMode::ThreadSafe> Stream)
{
	if (GetStatus() == EHttpRequestStatus::Processing)
	{
		return false;
	}
	
	StreamPayload = MakeUnique<FRequestPayloadInFileStream>(Stream);
	
	return FCurlHttpRequest::SetContentFromStream(Stream);
}

/**
 * Manually close a stream request and indicate it has ended
 */
void FWitHttpRequest::CloseStreamRequest()
{
	UE_LOG(LogWit, Verbose, TEXT("CloseStreamRequest: ending request"));

	IsEnded = true;
}

/**
 * Tick the request
 */
void FWitHttpRequest::Tick(float DeltaSeconds)
{
	FCurlHttpRequest::Tick(DeltaSeconds);

	// When using chunked transfer the request can get paused if the input stream is exhausted. We unpause it if there is 
	// sufficient data in the stream again

	if (!IsPaused)
	{
		return;
	}
	
	constexpr auto MaximumSizeBeforeUnpause = 4000;
	const size_t SizeUnsent = StreamPayload->GetContentLength() - StreamBytesSent.GetValue();
	const bool bShouldUnpause = (IsEnded || (SizeUnsent > MaximumSizeBeforeUnpause));
		
	if (bShouldUnpause)
	{
		UE_LOG(LogWit, Verbose, TEXT("Tick: resuming paused request"));
		
		curl_easy_pause(GetEasyHandle(), CURLPAUSE_CONT);
	
		IsPaused = false;
	}
}

/**
 * Get the user agent to use for Wit requests
 * If Voice SDK installed: voicesdk-unreal-49.0.1,wit-unreal-49.0.1,"Windows-10.0.19045.1.256.64bit","GenuineIntel|Intel(R)Core(TM)i7-10750HCPU@2.60GHz",C6C3C9D74AE98762558677BF627B5666,com.Meta.voicesdk_unreal_demo,Editor,4.27.2
 * If Voice SDK is NOT installed: wit-unreal-49.0.1,"Windows-10.0.19045.1.256.64bit","GenuineIntel|Intel(R)Core(TM)i7-10750HCPU@2.60GHz",C6C3C9D74AE98762558677BF627B5666,com.Meta.voicesdk_unreal_demo,Editor,4.27.2
 */
FString FWitHttpRequest::GetUserAgent()
{
	// OS

	const FString PlatformName = FString(FPlatformProperties::IniPlatformName());
	const FString OSVersion = FPlatformMisc::GetOSVersion();
	const FString OperatingSystem = FString::Printf(TEXT("%s-%s"), *PlatformName, *OSVersion);

	// Device - these need to be filled in with something sensible
	const FString DeviceModel = FPlatformMisc::GetDeviceMakeAndModel();

	// Unique GUID
	
	const FGuid Guid = FApp::GetSessionId();
	const FString SessionId = *Guid.ToString(EGuidFormats::Digits);

	// Additional Front User Data

	const FString AdditionalFrontUserData = FWitHelperUtilities::AdditionalFrontUserData;

	// SDK version

	const FString WitSdkVersion = FWitModule::Get().SdkVersion;
	const FString WitPlugin = FString::Printf(TEXT("wit-unreal-%s"), *WitSdkVersion);

	// Project

	const FString ProjectSettingsPath = TEXT("/Script/EngineSettings.GeneralProjectSettings");
	FString Company = FString::Printf(TEXT("YourCompany"));

	GConfig->GetString(
		*ProjectSettingsPath,
		TEXT("CompanyName"),
		Company,
		GGameIni
	);

	FString Project = FApp::GetProjectName();
	FString ProjectId = FString::Printf(TEXT("com.%s.%s"), *Company, *Project);

	// Editor
	
#if WITH_EDITOR
	const FString UserEnv = FString("Editor");
#else
	const FString UserEnv = FString("Runtime");
#endif

	const FString EditorVersion = ENGINE_VERSION_STRING;

	// Additional End User Data

	const FString AdditionalEndUserData = FWitHelperUtilities::AdditionalEndUserData;
	
	FString UserAgent = FString::Printf(TEXT("%s%s,\"%s\",\"%s\",%s,%s,%s,%s%s"),
		*FGenericPlatformHttp::EscapeUserAgentString(AdditionalFrontUserData),
		*FGenericPlatformHttp::EscapeUserAgentString(WitPlugin),
		*FGenericPlatformHttp::EscapeUserAgentString(OperatingSystem),
		*FGenericPlatformHttp::EscapeUserAgentString(DeviceModel),
		*FGenericPlatformHttp::EscapeUserAgentString(SessionId),
		*FGenericPlatformHttp::EscapeUserAgentString(ProjectId),
		*FGenericPlatformHttp::EscapeUserAgentString(UserEnv),
		*FGenericPlatformHttp::EscapeUserAgentString(EditorVersion),
		*FGenericPlatformHttp::EscapeUserAgentString(AdditionalEndUserData)
	);

	UE_LOG(LogWit, Verbose, TEXT("UserAgent: %s"), *UserAgent);

	return UserAgent;
}

/**
 * Start a threaded request
 */
bool FWitHttpRequest::StartThreadedRequest()
{
	SetupRequestOverrides();
	FCurlHttpRequest::StartThreadedRequest();

	return true;
}

/**
 * Setup any request overrides
 */
void FWitHttpRequest::SetupRequestOverrides()
{
	UE_LOG(LogWit, Verbose, TEXT("SetupRequestOverrides: trying to set overrides"));
	
	const bool bIsChunkedTransfer = GetHeader(TEXT("Transfer-Encoding")) == TEXT("chunked");
	const bool bIsPostRequest = GetVerb() == TEXT("POST");

	// Override the read function so we can implement pause and resume functionality for chunked transfers
	
	if (bIsPostRequest)
	{
		UE_LOG(LogWit, Verbose, TEXT("SetupRequestOverrides: overriding upload callback"));
		
		curl_easy_setopt(GetEasyHandle(), CURLOPT_READFUNCTION, StaticStreamUploadCallback);
	}
	
	// For chunked transfers we do not want to the the post field size and we need to strip the content length header
	
	if (bIsChunkedTransfer)
	{
		UE_LOG(LogWit, Verbose, TEXT("SetupRequestOverrides: overriding post size"));

		curl_easy_setopt(GetEasyHandle(), CURLOPT_POSTFIELDSIZE, -1);
		StripContentLengthHeader();
	}
}

/**
 * Regenerate the HTTP headers stripping out the content length header
 */
void FWitHttpRequest::StripContentLengthHeader()
{
	UE_LOG(LogWit, Verbose, TEXT("StripContentLengthHeader: trying to strip content length"));
	
	// Reset the headers but strip out the content length as it's not needed for chunked transfers
	
	curl_slist_free_all(StreamHeaderList);
	StreamHeaderList = nullptr;
	
	TArray<FString> AllHeaders = GetAllHeaders();

	for (const auto& Header : AllHeaders )
	{
		UE_LOG(LogWit, Verbose, TEXT("StripContentLengthHeader: header (%s)"), *Header);
		
		const bool bIsContentLengthHeader = Header.Contains(TEXT("Content-Length"));
		
		if (bIsContentLengthHeader)
		{
			UE_LOG(LogWit, Verbose, TEXT("StripContentLengthHeader: stripping content length"));
			
			continue;
		}
		
		curl_slist* AppendedHeaderList = curl_slist_append(StreamHeaderList, TCHAR_TO_UTF8(*Header));

		if (AppendedHeaderList)
		{
			StreamHeaderList = AppendedHeaderList;
		}
	}

	if (StreamHeaderList)
	{
		curl_easy_setopt(GetEasyHandle(), CURLOPT_HTTPHEADER, StreamHeaderList);
	}
}

/**
 * Replacement static upload callback to be used as a read function in curl
 */
size_t FWitHttpRequest::StaticStreamUploadCallback(void* Ptr, size_t SizeInBlocks, size_t BlockSizeInBytes, void* UserData)
{
	FWitHttpRequest* Request = static_cast<FWitHttpRequest*>(UserData);
	return Request->StreamUploadCallback(Ptr, SizeInBlocks, BlockSizeInBytes);
}

/**
 * Replacement upload callback called from the static upload callback
 */
size_t FWitHttpRequest::StreamUploadCallback(void* Ptr, size_t SizeInBlocks, size_t BlockSizeInBytes)
{
	const size_t SizeAlreadySent = static_cast<size_t>(StreamBytesSent.GetValue());
	const size_t SizeSentNow = StreamPayload->FillOutputBuffer(Ptr, SizeInBlocks * BlockSizeInBytes, SizeAlreadySent);
	
	StreamBytesSent.Add(SizeSentNow);
	
	// If we exhaust the input stream then pause the request to wait for more

	const bool IsInputStreamExhausted = ( !IsEnded && (SizeSentNow == 0 ));

	if (IsInputStreamExhausted)
	{
		UE_LOG(LogWit, Verbose, TEXT("StreamUploadCallback: data is exhausted - pausing request"));
		
		IsPaused = true;
		
		return CURL_READFUNC_PAUSE;
	}
	
	return SizeSentNow;
}

#if WITH_EDITOR

/**
 * Construct a stream payload from an archive 
 */
FRequestPayloadInFileStream::FRequestPayloadInFileStream(TSharedRef<FArchive, ESPMode::ThreadSafe> InFile) : File(InFile)
{
	// Deliberately empty
}

/**
 * Destructor
 */
FRequestPayloadInFileStream::~FRequestPayloadInFileStream()
{
	// Deliberately empty
}

/**
 * Get the content length of the payload
 */
int32 FRequestPayloadInFileStream::GetContentLength() const
{
	return static_cast<int32>(File->TotalSize());
}

/**
 * Get access to the payload content
 */
const TArray<uint8>& FRequestPayloadInFileStream::GetContent() const
{
	static const TArray<uint8> NotSupported;
	return NotSupported;
}

/**
 * Is ths URL encoded or not?
 */
bool FRequestPayloadInFileStream::IsURLEncoded() const
{
	// Deliberately empty
	
	return false;
}

/**
 * Given an output buffer fill it with data from the stream
 */
size_t FRequestPayloadInFileStream::FillOutputBuffer(void* OutputBuffer, size_t MaxOutputBufferSize, size_t SizeAlreadySent)
{
	return FillOutputBuffer(TArrayView<uint8>(static_cast<uint8*>(OutputBuffer), MaxOutputBufferSize), SizeAlreadySent);
}

/**
 * Given an output buffer fill it with data from the stream
 */
size_t FRequestPayloadInFileStream::FillOutputBuffer(TArrayView<uint8> OutputBuffer, size_t SizeAlreadySent)
{
	const size_t ContentLength = static_cast<size_t>(GetContentLength());
	const size_t SizeLeftToSend = ContentLength - SizeAlreadySent;
	const size_t SizeToSendNow = FMath::Min(SizeLeftToSend, static_cast<size_t>(OutputBuffer.Num()));
	
	if (SizeToSendNow != 0)
	{		
		File->Serialize(OutputBuffer.GetData(), static_cast<int64>(SizeToSendNow));
	}
	
	return SizeToSendNow;
}

/**
 * Construct an memory payload from an array directly
 */
FRequestPayloadInMemory::FRequestPayloadInMemory(const TArray<uint8>& Array) : Buffer(Array)
{
	// Deliberately empty
}

/**
 * Construct a memory payload from an array copy
 */
FRequestPayloadInMemory::FRequestPayloadInMemory(TArray<uint8>&& Array) : Buffer(MoveTemp(Array))
{
	// Deliberately empty
}

/**
 * Destructor
 */
FRequestPayloadInMemory::~FRequestPayloadInMemory()
{
	// Deliberately empty
}

/**
 * * Get the content length of the payload
 */
int32 FRequestPayloadInMemory::GetContentLength() const
{
	return Buffer.Num();
}

/**
 * Get access to the content payload
 */
const TArray<uint8>& FRequestPayloadInMemory::GetContent() const
{
	return Buffer;
}

/**
 * Is the URL encoded or not?
 */
bool FRequestPayloadInMemory::IsURLEncoded() const
{
	// Deliberately empty
	
	return false;
}

/**
 * Given an output buffer fill it with data from the stream
 */
size_t FRequestPayloadInMemory::FillOutputBuffer(void* OutputBuffer, size_t MaxOutputBufferSize, size_t SizeAlreadySent)
{
	// Deliberately empty
	
	return 0;
}

/**
 * Given an output buffer fill it with data from the stream
 */
size_t FRequestPayloadInMemory::FillOutputBuffer(TArrayView<uint8> OutputBuffer, size_t SizeAlreadySent)
{
	// Deliberately empty
	
	return 0;
}

#endif

#endif
