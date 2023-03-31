/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#if WITH_CURL

#include "CoreMinimal.h"
#include "Curl/CurlHttp.h"

class FWitHttpResponse;

/**
 * Extend and modify Unreal's default Curl request to support chunked transfers
 */
class FWitHttpRequest : public FCurlHttpRequest
{
public:

	friend class FWitHttpResponse;

	/**
	 * Constructor
	 */
	FWitHttpRequest() = default;

	/**
	 * Destructor. Clean up any connection/request handles
	 */
	virtual ~FWitHttpRequest() override;

	/**
	 * IHttpRequest overrides
	 */
	virtual bool SetContentFromStream(TSharedRef<FArchive, ESPMode::ThreadSafe> Stream) override;
	virtual void Tick(float DeltaSeconds) override;

	/**
	 * IHttpRequestThreaded overrides
	 */
	virtual bool StartThreadedRequest() override;
	
	/**
	 * Manually close a stream request and indicate it has ended
	 */
	void CloseStreamRequest();

	/**
	 * Setup any request overrides
	 */
	void SetupRequestOverrides();

	/**
	 * Replacement static upload callback to be used as a read function in curl
	 */
	static size_t StaticStreamUploadCallback(void* Ptr, size_t SizeInBlocks, size_t BlockSizeInBytes, void* UserData);

	/**
	 * Replacement upload callback called from the static upload callback
	 */
	size_t StreamUploadCallback(void* Ptr, size_t SizeInBlocks, size_t BlockSizeInBytes);

	/*
	 * User agent for Wit
	 */
	static FString GetUserAgent();

private:

	/** Strip the content length header */
	void StripContentLengthHeader();
	
	/** Has the request been paused? */
	bool IsPaused{ false };

	/** Has the request ended? */
	bool IsEnded{ false };

	/** The payload we want to stream with the request */
	TUniquePtr<FRequestPayload> StreamPayload;

	/** Count of bytes that we have streamed so far */
	FThreadSafeCounter StreamBytesSent;

	/** List of custom headers to be passed to CURL */
	curl_slist*	StreamHeaderList{nullptr};
};

#endif
