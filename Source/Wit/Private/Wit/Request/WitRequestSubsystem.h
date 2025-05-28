/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "Serialization/BufferArchive.h"
#include "Wit/Request/WitRequestConfiguration.h"
#include "Subsystems/EngineSubsystem.h"
#include "Serialization/MemoryReader.h"
#include "Misc/EngineVersionComparison.h"
#include "WitRequestSubsystem.generated.h"

class FJsonObject;
class FMemoryReader;
class FSubsystemCollectionBase;

/**
 * A class to track an in progress Wit.ai request. It essentially wraps a UE4 HTTP request
 * while also providing a streaming read buffer
 */
UCLASS()
class UWitRequestSubsystem final : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:

	/**
	 * Initialize the subsystem. USubsystem override
	 *
	 * @return true if successfully initialized
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * De-initializes the subsystem. USubsystem override
	 *
	 * @return true if successfully initialized
	 */
	virtual void Deinitialize() override;

	/**
	 * Start a Wit.ai request. Depending on the request endpoint this may be a streaming request or not. Only
	 * the /speech endpoint supports streaming. This should always be paired with a call to EndStreamRequest
	 *
	 * @param RequestConfiguration [in] The configuration to use to setup the request
	 */
	void BeginStreamRequest(const FWitRequestConfiguration& RequestConfiguration);

	/**
	 * Finish a Wit.ai request. In the case of a streaming request this should be called when there is no more
	 * data to send. In the case of one shot requests it can be called immediately after BeginStreamRequest
	 */
	void EndStreamRequest();

	/**
	 * Cancels an inflight Wit.ai request
	 */
	void CancelRequest();

	/**
	 * Is a Wit.ai request currently in progress?
	 *
	 * @return true if a request is in progress
	 */
	bool IsRequestInProgress() const;

	/**
	 * Writes the given binary data to the internal stream that the request is using
	 *
	 * @param Data [in] the content to add to the stream buffer
	 */
	void WriteBinaryData(const TArray<uint8>& Data);

	/**
	 * Writes the given Json data to the internal stream that the request is using
	 *
	 * @param Data [in] the content to add to the stream buffer
	 */
	void WriteJsonData(const TSharedRef<FJsonObject> Data);

private:

	/** Actually sends the HTTP request */
	void SendRequest();

	/** Called when an HTTP request is in progress to retrieve any changes to the response payload */
#if UE_VERSION_OLDER_THAN(5, 4, 0)
    void OnRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived);
#else
    void OnRequestProgress(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived);
#endif

	/** Called when an HTTP request is fully completed to process the response payload */
	void OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bIsSuccessful);
	
	/** Splits a response JSON string into chunks as defined by the Wit.ai response format */
	static void SplitResponseIntoChunks(const FString& Response, TArray<FString>& ChunkedResponses);

	/** Used to track if a configuration has been set or not */
	bool bHasConfiguration{false};

	/** The current configuration setup */
	FWitRequestConfiguration Configuration{};

	/** The underlying UE4 HTTP request that is used to process the Wit.ai request */
	FHttpRequestPtr HttpRequest{nullptr};

	/** The raw content data that makes up the body of a POST request */
	TArray<uint8> ContentStream{};

	/** Wraps the ContentStream to provide an FArchive interface for a streaming Wit.ai request */
	TSharedPtr<FMemoryReader, ESPMode::ThreadSafe> MemoryReader{};

	/** The most recently received response length */
	int32 LastResponseSize{0};
};
