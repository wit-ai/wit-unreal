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
#include "WitSocketSubsystem.generated.h"



// Define a struct to represent the encoded data
struct EncodedData
{
	std::string JsonData;
	std::vector<uint8_t> BinaryData;
};

/**
 * Type of request being made to Wit.ai 
 */
UENUM()
enum class ERequestType : uint8
{
	Synthesize,
	Converse,
};

/**
 * State of the WebSocket connection
 */
UENUM()
enum class ESocketState : uint8
{
	Disconnected,
	Connecting,
	Connected,
	Authenticating,
	Authenticated,
};

DECLARE_MULTICAST_DELEGATE(FSocketStatusDelegate);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWitSocketErrorDelegate, const FString&, const FString&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWitSocketProgressDelegate, const TArray<uint8>&, const TSharedPtr<FJsonObject>);
DECLARE_MULTICAST_DELEGATE(FOnWitSocketCompleteDelegate);

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
	 * Close a WebSocket connection
	 */
	void CloseSocket();

	/**
	 * Checks if there is an active synthesize request in progress
	 *
	 * @return Is there a synethesize currently in progress
	 */
	bool IsSynthesizeInProgress();

	/**
	 * Checks if there is an active converse request in progress
	 *
	 * @return Is there a converse currently in progress
	 */
	bool IsConverseInProgress();

	/**
	 * Checks the current status of the WebSocket connection
	 *
	 * @return Current state of the WebSocket connection
	 */
	ESocketState GetSocketState();

	/**
	 * Send binary data across an active WebSocket connection
	 *
	 * @param Data [in] binary data to send over connection
	 */
	void SendBinaryData(const TArray<uint8>& Data);

	/**
	 * Send JSON data across an active WebSocket connection
	 *
	 * @param Type [in] Type of request to send over connection
	 * @param Data [in] JSON data to send over connection
	 */
	void SendJsonData(const ERequestType Type, const TSharedRef<FJsonObject> Data);

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

	/** Is there an active Converse request in progress */
	bool bConverseInProgress;

	/** Current id of request in progress */
	FString RequestId;

	/** Current status of the WebSocket connection */
	ESocketState SocketStatus;

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
