/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>
#include <vector>
#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "Subsystems/EngineSubsystem.h"
#include "Wit/Request/WitRequestConfiguration.h"
#include "Wit/TTS/WitTtsService.h"
#include "WitSocketSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FSocketStatusDelegate, const SocketState State);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWitSocketErrorDelegate, const FString&, const FString&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWitSocketProgressDelegate, const TArray<uint8>&, const TSharedPtr<FJsonObject>);
DECLARE_MULTICAST_DELEGATE(FOnWitSocketCompleteDelegate);

// Define a struct to represent the encoded data
struct EncodedData {
	std::string JsonData;
	std::vector<uint8_t> BinaryData;
};

/**
 * A class to handle WebSocket connections for in progress Wit.ai requests.
 */
UCLASS()
class UWitSocketSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Initialize the subsystem. USubsystem override
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * De-initializes the subsystem. USubsystem override
	 */
	virtual void Deinitialize() override;

	/**
	 * Create a WebSocket connection
	 *
	 * @param AuthToken [in] Authentication token used to establish a WebSocket connection
	 */
	void CreateSocket(const FString AuthToken);

	/**
	 * Checks if there is an active synthesize request in progress
	 *
	 * @return Is there a synethesize currently in progress
	 */
	bool IsSynthesizeInProgress();

	/**
	 * Send JSON data across an active WebSocket connection
	 *
	 * @param Data [in] JSON data to send over connection
	 */
	void SendJsonData(const TSharedRef<FJsonObject> Data);

	/** Callback to use when the WebSocket status changes */
	FSocketStatusDelegate OnSocketStateChange;

	/** Optional callback to use when the request errors */
	FOnWitSocketErrorDelegate OnSocketStreamError{};

	/** Optional callback to use when the request is in progress */
	FOnWitSocketProgressDelegate OnSocketStreamProgress{};

	/** Optional callback to use when the request is complete */
	FOnWitSocketCompleteDelegate OnSocketStreamComplete{};

private:
	/** URL of the Wit.ai server to connect to */
	const FString ServerURL = TEXT("wss://api.wit.ai/composer");

	/** Server protocol to use for WebSocket conection */
	const FString ServerProtocol = TEXT("wss");

	/** WebSocket connection */
	TSharedPtr<IWebSocket> Socket;

	/** Is the WebSocket connection authenticated */
	bool bWebSocketAuthenticated;

	/** Is there an active Synthesize request in progress */
	bool bSynthesizeInProgress;

	/**
	 * Encodes data to send over the WebSocket connection
	 *
	 * @param JsonData [in] JSON data to send over the WebSocket connection
	 * @param BinaryData [in] binary data to send over the WebSocket connection
	 */
	std::string Encode(
		const std::string& JsonData,
		const std::vector<unsigned char>& BinaryData = {});

	/**
	 * Decodes data received over the WebSocket connection
	 *
	 * @param Message [in] data returned from WebSocket connection
	 * @return Data from WebSocket parsed into an EncodedData struct
	 */
	EncodedData static Decode(const std::string Message);
};
