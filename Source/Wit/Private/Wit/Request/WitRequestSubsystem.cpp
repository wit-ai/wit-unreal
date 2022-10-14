/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Request/WitRequestSubsystem.h"
#include "Wit/Utilities/WitLog.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Wit/Request/HTTP/WitHttpRequest.h"

/**
 * Initialize the subsystem. USubsystem override
 */
void UWitRequestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	MemoryReader = MakeShared<FMemoryReader, ESPMode::ThreadSafe>(ContentStream);
}

/**
 * De-initializes the subsystem. USubsystem override
 */
void UWitRequestSubsystem::Deinitialize()
{
	
}

/**
 * Start a Wit.ai request. Depending on the request endpoint this may be a streaming request or not. Only
 * the /speech endpoint supports streaming. This should always be paired with a call to EndStreamRequest
 *
 * @param RequestConfiguration [in] the configuration to use to setup the request
 */
void UWitRequestSubsystem::BeginStreamRequest(const FWitRequestConfiguration& RequestConfiguration)
{
	if (IsRequestInProgress())
	{
		UE_LOG(LogWit, Warning, TEXT("BeginRequest: Attempting to begin request when one is already in progress"));
		return;
	}

	ContentStream.Reset();
	
	MemoryReader->Reset();
	MemoryReader->Seek(0);

	Configuration = RequestConfiguration;
	LastResponseSize = 0;
	bHasConfiguration = true;

	// When streaming we start the request immediately. Data will be passed to the server as it becomes available
	
	if (RequestConfiguration.bShouldUseChunkedTransfer)
	{
		SendRequest();
	}
}

/**
 * Finish a Wit.ai request. In the case of a streaming request this should be called when there is no more
 * data to send. In the case of one shot requests it can be called immediately after BeginStreamRequest
 */
void UWitRequestSubsystem::EndStreamRequest()
{
	MemoryReader->Close();
	
	if (Configuration.bShouldUseChunkedTransfer)
	{
		const TSharedPtr<FWitHttpRequest, ESPMode::ThreadSafe> StreamRequest = StaticCastSharedPtr<FWitHttpRequest, IHttpRequest>(HttpRequest);
		StreamRequest->CloseStreamRequest();
	}
	else
	{
		SendRequest();
	}
}

/**
 * Actually sends the HTTP request
 */
void UWitRequestSubsystem::SendRequest()
{
	if (IsRequestInProgress())
	{
		UE_LOG(LogWit, Warning, TEXT("SendRequest: Attempting to process request when one is already in progress"));
		return;
	}

	if (!bHasConfiguration)
	{
		UE_LOG(LogWit, Warning, TEXT("SendRequest: No configuration has been specified for the request"));
		return;
	}

	// If we are using streaming then we use our custom HTTP request otherwise we fallback to UE4's standard HTTP request

	HttpRequest = TSharedRef<IHttpRequest, ESPMode::ThreadSafe>(dynamic_cast<IHttpRequest*>(new FWitHttpRequest()));

	// Construct the final URL for the request
	
	FString Url = FString::Format(TEXT("{0}/{1}"), { Configuration.BaseUrl, Configuration.Endpoint });

	const bool bIsVersionParameter = !Configuration.Version.IsEmpty();
	const bool bIsUrlParameters = bIsVersionParameter || Configuration.Parameters.Num() > 0;

	if (bIsUrlParameters)
	{
		Url.Append("?");
	}
	
	if (bIsVersionParameter)
	{
		Url.Append(TEXT("v="));
		Url.Append(Configuration.Version);
	}

	for (const TPair<FString, FString >& ParameterPair : Configuration.Parameters)
	{
		Url.Append(ParameterPair.Key);
		Url.Append(ParameterPair.Value);
	}

	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(Configuration.Verb);

	// Add headers. This varies per endpoint but all requests require the Authorization header
	
	const FString Authorization = FString::Format(TEXT("Bearer {0}"), { Configuration.AuthToken });

	HttpRequest->SetHeader("Authorization", Authorization);
	HttpRequest->SetHeader("User-Agent", FWitHttpRequest::GetUserAgent());

	if (!Configuration.Accept.IsEmpty())
	{
		HttpRequest->SetHeader("Accept", Configuration.Accept);
	}
	
	FString ContentType;
	bool bIsSeparatorRequired = false;
	
	for (const TPair<FString, FString >& ContentTypePair : Configuration.ContentTypes)
	{
		if (bIsSeparatorRequired)
		{
			ContentType.Append(";");
		}
		else
		{
			bIsSeparatorRequired = true;
		}
		
		ContentType.Append(ContentTypePair.Key);
		ContentType.Append(ContentTypePair.Value);		
	}

	if (!ContentType.IsEmpty())
	{
		HttpRequest->SetHeader("Content-Type", ContentType);
	}
	
	if (Configuration.bShouldUseChunkedTransfer)
	{
		HttpRequest->SetHeader("Transfer-Encoding", TEXT("chunked"));
	}

	// Add body content. This can be either streamed or fixed depending on the endpoint
	
	HttpRequest->SetContentFromStream(MemoryReader.ToSharedRef());

	// Setup callbacks to inform of request progress and request completion

	HttpRequest->OnRequestProgress().BindUObject(this, &UWitRequestSubsystem::OnRequestProgress);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWitRequestSubsystem::OnRequestComplete);

	// Set custom timeout

	if (Configuration.bShouldUseCustomHttpTimeout)
	{
		UE_LOG(LogWit, Verbose, TEXT("SendRequest: Setting custom timeout to (%f)"), Configuration.HttpTimeout);

		HttpRequest->SetTimeout(Configuration.HttpTimeout);	
	}

	// Finally send off the request
	
	if (HttpRequest != nullptr)
	{
		HttpRequest->ProcessRequest();

		UE_LOG(LogWit, Verbose, TEXT("SendRequest: Url is (%s), Content type is (%s) and Content length is (%d)"), *HttpRequest->GetURL(), *HttpRequest->GetHeader("Content-Type"), ContentStream.Num());
	}
	else
	{
		UE_LOG(LogWit, Warning, TEXT("SendRequest: failed"));
	}
}

/**
 * Writes the given data to the internal stream that the request is using
 *
 * @param Data [in] the content to add to the stream buffer
 */
void UWitRequestSubsystem::WriteBinaryData(const TArray<uint8>& Data)
{
	const int32 NumBytesToCopy = Data.Num();
	
	if (NumBytesToCopy <= 0)
	{
		return;
	}

	UE_LOG(LogWit, Verbose, TEXT("WriteBinaryData: Old reader size is (%lld)"), MemoryReader->TotalSize());

	const int32 Offset = ContentStream.AddUninitialized(NumBytesToCopy);
	const uint8* CopyFrom = Data.GetData();
	uint8* CopyTo = ContentStream.GetData() + Offset;

	FMemory::Memcpy(CopyTo, CopyFrom, NumBytesToCopy);

	UE_LOG(LogWit, Verbose, TEXT("WriteBinaryData: Wrote (%d) bytes. New array size is (%d) New reader size is (%lld)"), NumBytesToCopy, ContentStream.Num(), MemoryReader->TotalSize());
}

/**
 * Writes the given data to the internal stream that the request is using
 *
 * @param Data [in] the content to add to the stream buffer
 */
void UWitRequestSubsystem::WriteJsonData(const TSharedRef<FJsonObject> Data)
{
	// Stringify the Json object
	
	FString ContentString;
	const TSharedRef<TJsonWriter<TCHAR>> Writer = TJsonWriterFactory<TCHAR>::Create(&ContentString);
	
	FJsonSerializer::Serialize(Data, Writer);

	// Convert the string to UTF8 and copy in place
	
	const int32 NumBytesToCopy = FTCHARToUTF8_Convert::ConvertedLength(*ContentString, ContentString.Len());

	if (NumBytesToCopy <= 0)
	{
		return;
	}

	UE_LOG(LogWit, Verbose, TEXT("WriteJsonData: Old reader size is (%lld)"), MemoryReader->TotalSize());

	const int32 Offset = ContentStream.AddUninitialized(NumBytesToCopy);
	uint8* CopyTo = ContentStream.GetData() + Offset;

	FTCHARToUTF8_Convert::Convert(reinterpret_cast<ANSICHAR*>(CopyTo), NumBytesToCopy, *ContentString, ContentString.Len());

	UE_LOG(LogWit, Verbose, TEXT("WriteJsonData: Wrote (%d) bytes. New array size is (%d) New reader size is (%lld)"), NumBytesToCopy, ContentStream.Num(), MemoryReader->TotalSize());
}

/**
 * Cancels an inflight Wit.ai request
 */
void UWitRequestSubsystem::CancelRequest()
{
	if (!IsRequestInProgress())
	{
		return;
	}

	HttpRequest->CancelRequest();
	HttpRequest = nullptr;
}

/**
 * Is a Wit.ai request currently in progress?
 *
 * @return true if a request is in progress
 */
bool UWitRequestSubsystem::IsRequestInProgress() const
{
	return HttpRequest != nullptr;
}

/**
 * Called when an HTTP request is in progress to retrieve any changes to the response payload
 *
 * @param Request the in progress request
 * @param BytesSent the amount of bytes that have so far been sent to server
 * @param BytesReceived the amount of bytes that have so far been received from server
 */
void UWitRequestSubsystem::OnRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
{
	if (!Configuration.OnRequestProgress.IsBound())
	{
		return;	
	}
	
	const TArray<uint8>& ContentAsBytes(Request->GetResponse()->GetContent());
	const bool bIsNewResponseData = ContentAsBytes.Num() != LastResponseSize;

	if (!bIsNewResponseData)
	{
		UE_LOG(LogWit, Verbose, TEXT("OnRequestProgress: Ignoring response progress because size has not changed"));
		return;	
	}
	
	UE_LOG(LogWit, Verbose, TEXT("OnRequestProgress: Content size (%d) bytes received (%d)"), ContentAsBytes.Num(), BytesReceived);

	const FUTF8ToTCHAR ContentAsTChar(reinterpret_cast<const ANSICHAR*>(ContentAsBytes.GetData()), ContentAsBytes.Num());
	const FString Content(ContentAsTChar.Length(), ContentAsTChar.Get());

	UE_LOG(LogWit, Verbose, TEXT("OnRequestProgress: Content as string (%s)"), *Content);

	// The speech endpoint returns chunked responses which contain multiple JSON objects. The final chunk represents the most recent response (at this time)
	// while the other chunks are intermediate results that can be safely ignored

	TArray<FString> ChunkedResponses;

	SplitResponseIntoChunks(Content, ChunkedResponses);

	const bool bIsMalformedResponse = (ChunkedResponses.Num() == 0);
	if (bIsMalformedResponse)
	{
		return;
	}

	const FString FinalResponse{ChunkedResponses.Last()};

	TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
	const TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(FinalResponse);

	const bool bIsDeserializationError = !FJsonSerializer::Deserialize(Reader, Json);
	if (bIsDeserializationError)
	{
		return;
	}

	const bool bIsValidPartialTranscription = Json->HasField("text");
	if (!bIsValidPartialTranscription)
	{
		return;
	}

	LastResponseSize = ContentAsBytes.Num();
			
	Configuration.OnRequestProgress.Broadcast(ContentAsBytes, Json);
}

/**
 * Called when an HTTP request is fully completed to process the response payload
 *
 * @param Request the completed request
 * @param Response the full and final response
 * @param bIsSuccessful whether the request successfully completed
 */
void UWitRequestSubsystem::OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bIsSuccessful)
{
	HttpRequest = nullptr;
	
	if (!bIsSuccessful)
	{
		const FString ErrorMessage = FString::Format(TEXT("HTTP Error {0}"), { Response->GetResponseCode()});
		const FString HumanReadableErrorMessage = FString::Format(TEXT("Request failed with error code {0}"), { Response->GetResponseCode()});
		
		Configuration.OnRequestError.Broadcast(ErrorMessage, HumanReadableErrorMessage);
		
		return;
	}

	const FString Content = Response->GetContentAsString();
	const FString ContentType = Response->GetContentType();

	const bool bIsJsonContentType = ContentType.Contains(TEXT("application/json"));
	const bool bIsAudioContentType = ContentType.Contains(TEXT("audio/wav"));

	UE_LOG(LogWit, Verbose, TEXT("OnRequestComplete: Content as string (%s)"), *Content);

	if (bIsJsonContentType)
	{
		// The speech endpoint returns chunked responses which contain multiple JSON objects. The final chunk represents the final response to the
		// entire request while the other chunks are intermediate results that can be safely ignored

		TArray<FString> ChunkedResponses;

		SplitResponseIntoChunks(Content, ChunkedResponses);

		const bool bIsMalformedResponse = (ChunkedResponses.Num() == 0);
		if (bIsMalformedResponse)
		{
			Configuration.OnRequestError.Broadcast(TEXT("Invalid response"), TEXT("Response is incomplete or otherwise invalid"));
			return;
		}

		const FString FinalResponse{ChunkedResponses.Last()};

		TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject());
		const TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(FinalResponse);

		const bool bIsDeserializationError = !FJsonSerializer::Deserialize(Reader, Json);
		if (bIsDeserializationError)
		{
			Configuration.OnRequestError.Broadcast(TEXT("Deserialization failed"), TEXT("Deserializing the response to JSON failed"));
			return;
		}

		UE_LOG(LogWit, Verbose, TEXT("OnRequestComplete: calling delegate"));
		
		Configuration.OnRequestComplete.Broadcast(Response->GetContent(), Json);
	}
	else if (bIsAudioContentType)
	{
		// The synthesize endpoint returns binary data in the form of a wav
		
		Configuration.OnRequestComplete.Broadcast(Response->GetContent(), nullptr);
	}
	else
	{
		Configuration.OnRequestError.Broadcast(TEXT("Invalid content type"), TEXT("Response has invalid content type"));
	}
}

/**
 * Splits a response JSON string into chunks as defined by the Wit.ai response format
 *
 * @param Response the string version of the response
 * @param ChunkedResponses the response broken down into brace delimited chunks
 */
void UWitRequestSubsystem::SplitResponseIntoChunks(const FString& Response, TArray<FString>& ChunkedResponses)
{
	int32 StringIndex = 0;

	ChunkedResponses.Reset();

	// The speech endpoint implements chunked responses by using a JSON format that does not strictly conform to the JSON specification.
	// Because of this UE4's JSON object converter cannot handle it without some preprocessing to break each chunk down into its own
	// conforming chunk. See the Wit.ai documentation for the specifics of the response format; it consists of a sequence of brace
	// delimited JSON objects

	while (StringIndex < Response.Len())
	{
		const int32 OpeningBraceIndex = Response.Find(TEXT("{"), ESearchCase::IgnoreCase, ESearchDir::FromStart, StringIndex);

		const bool bIsStartOfNewChunk = (OpeningBraceIndex != INDEX_NONE);
		if (!bIsStartOfNewChunk)
		{
			return;
		}

		StringIndex = OpeningBraceIndex + 1;
		int32 BraceCount = 1;

		while (BraceCount > 0 && StringIndex < Response.Len())
		{
			if (Response[StringIndex] == L'{')
			{
				++BraceCount;
			}
			else if (Response[StringIndex] == L'}')
			{
				--BraceCount;
			}

			++StringIndex;
		}

		FString ChunkedResponseString{Response.Mid(OpeningBraceIndex, StringIndex - OpeningBraceIndex)};

		ChunkedResponses.Add(ChunkedResponseString);

		UE_LOG(LogWit, VeryVerbose, TEXT("Chunk string found (%s)"), *ChunkedResponseString);
	}
}
