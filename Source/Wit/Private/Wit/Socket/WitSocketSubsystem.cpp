/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Socket/WitSocketSubsystem.h"
#include "Dom/JsonObject.h"
#include "Misc/ByteSwap.h"
#include "Misc/Guid.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "WebSocketsModule.h"
#include "Wit/Utilities/WitLog.h"
#include <string>

 /**
  * Initialize the subsystem. USubsystem override
  */
void UWitSocketSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}

/**
 * De-initializes the subsystem. USubsystem override
 */
void UWitSocketSubsystem::Deinitialize()
{
	if (Socket)
	{
		Socket->Close();
	}
}

/**
 * Create a WebSocket connection
 *
 * @param AuthToken [in] Authentication token used to establish a WebSocket connection
 */
void UWitSocketSubsystem::CreateSocket(const FString AuthToken)
{
	if (Socket && Socket->IsConnected())
	{
		UE_LOG(LogWit, Warning, TEXT("CreateSocket: Socket already connected"));
		return;
	}

	Socket = FWebSocketsModule::Get().CreateWebSocket(ServerURL, ServerProtocol);

	Socket->OnConnected().AddLambda([this, AuthToken]() -> void
		{
			UE_LOG(LogWit, Display, TEXT("WebSockets: Connection Success"));

			std::string Encoded = Encode(std::string(("{\"wit_auth_token\": \"" + std::string(TCHAR_TO_UTF8(*AuthToken)) + "\"}")));

			const bool bIsBinary = true;
			SocketStatus = ESocketState::Connected;
			OnSocketStateChange.Broadcast();
			Socket->Send(reinterpret_cast<const uint8_t*>(&Encoded[0]), Encoded.length(), bIsBinary);
			SocketStatus = ESocketState::Authenticating;
			OnSocketStateChange.Broadcast();
		});

	Socket->OnConnectionError().AddLambda([this](const FString& Error) -> void
		{
			std::string ErrorText = std::string(TCHAR_TO_UTF8(*Error));
			UE_LOG(LogWit, Warning, TEXT("WebSockets: Connection Failed: %s"), *FString(ErrorText.c_str()));

			SocketStatus = ESocketState::Disconnected;
			OnSocketStateChange.Broadcast();
		});

	Socket->OnClosed().AddLambda(
		[this](int32 StatusCode, const FString& Reason, bool bWasClean) -> void
		{
			std::string ErrorText = std::string(TCHAR_TO_UTF8(*Reason));
			UE_LOG(LogWit, Display, TEXT("WebSockets: Connection Closed: %d %s"), StatusCode, *FString(ErrorText.c_str()));
			bWebSocketAuthenticated = false;
			SocketStatus = ESocketState::Disconnected;
			OnSocketStateChange.Broadcast();
		});

	Socket->OnMessage().AddLambda([](const FString& Message) -> void
		{
			/* Purposefully empty as all responses are binary */
		});

	Socket->OnRawMessage().AddLambda(
		[this](const void* Data, SIZE_T Size, SIZE_T BytesRemaining) -> void
		{
			UE_LOG(LogWit, Verbose, TEXT("WebSockets: Binary message received"));
			TArray<uint8> DataArray;
			DataArray.Append((const uint8*)Data, Size);

			FString DataString;
			std::string ResultCstr;
			for (int i = 0; i < DataArray.Num(); i++)
			{
				DataString.AppendChar(DataArray[i]);
				ResultCstr += DataArray[i];
			}

			EncodedData Encoded;
			if (!bSynthesizeInProgress)
			{
				Encoded = Decode(ResultCstr);
			}

			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(&Encoded.JsonData[0]);
			if (!FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				UE_LOG(LogWit, Warning, TEXT("WebSockets: Could not parse JSON %s"), *DataString);
			}

			FString ResponseType;
			if (JsonObject)
			{
				UE_LOG(LogWit, Verbose, TEXT("WebSockets: Getting response type"));
				ResponseType = JsonObject->GetStringField("type");
			}

			if (DataString.Contains("SYNTHESIZE_DATA"))
			{
				DataArray.Reset(0);
				DataArray.Append(&Encoded.BinaryData[0], (Size < Encoded.BinaryData.size()) ? Size : Encoded.BinaryData.size());
				bSynthesizeInProgress = true;
			}
			else if (!ResponseType.IsEmpty() && ResponseType.Equals("INITIALIZED"))
			{
				UE_LOG(LogWit, Verbose, TEXT("WebSockets: Initialized: %s"), *DataString);
				DataArray.Reset(0);
				bConverseInProgress = true;
			}
			else if (!ResponseType.IsEmpty() && ResponseType.Equals("EXECUTION_RESULT"))
			{
				UE_LOG(LogWit, Verbose, TEXT("WebSockets: Result: %s"), *DataString);
				bWebSocketAuthenticated = true;
				SocketStatus = ESocketState::Authenticated;
				OnSocketStateChange.Broadcast();
			}
			else if (DataString.Contains("END_STREAM"))
			{
				UE_LOG(LogWit, Verbose, TEXT("WebSockets: Synthesize Ended: %s"), *DataString);
				bSynthesizeInProgress = false;
				OnSocketStreamComplete.Broadcast();
			}
			else if (!ResponseType.IsEmpty() && ResponseType.Equals("END_TRANSCRIPTION"))
			{
				UE_LOG(LogWit, Verbose, TEXT("WebSockets: Synthesize Ended: %s"), *DataString);
				bConverseInProgress = false;
				OnSocketStreamComplete.Broadcast();
			}
			else if (!ResponseType.IsEmpty() && ResponseType.Equals("PARTIAL_TRANSCRIPTION"))
			{
				UE_LOG(LogWit, Verbose, TEXT("WebSockets: Partial Transcription: %s"), *DataString);
				TArray<unsigned char> BinaryData;
				BinaryData.Append(&Encoded.BinaryData[0], Encoded.BinaryData.size());

				OnSocketStreamProgress.Broadcast(BinaryData, JsonObject);			
			}
			else if (!ResponseType.IsEmpty() && ResponseType.Equals("FINAL_TRANSCRIPTION"))
			{
				UE_LOG(LogWit, Verbose, TEXT("WebSockets: Transcription Ended: %s"), *DataString);
				bSynthesizeInProgress = false;
				OnSocketStreamComplete.Broadcast();
			}
			else
			{
				UE_LOG(LogWit, Verbose, TEXT("WebSockets: Undefined message: %s"), *DataString);
			}
			if (bSynthesizeInProgress || (bConverseInProgress && DataArray.Num() > 0))
			{
				OnSocketStreamProgress.Broadcast(DataArray, nullptr);
			}

		});

	Socket->OnMessageSent().AddLambda([](const FString& MessageString) -> void
		{
			UE_LOG(LogWit, Verbose, TEXT("WebSockets: Message sent %s"), *MessageString);
		});

	Socket->Connect();
	SocketStatus = ESocketState::Connecting;
	OnSocketStateChange.Broadcast();
}

void UWitSocketSubsystem::CloseSocket()
{
	if (Socket && Socket->IsConnected())
	{
		Socket->Close();
	}
	bSynthesizeInProgress = false;
	bConverseInProgress = false;
}

/**
 * Checks if there is an active synthesize request in progress
 *
 * @return Is there a synethesize currently in progress
 */
bool UWitSocketSubsystem::IsSynthesizeInProgress()
{
	return bSynthesizeInProgress;
}

/**
 * Checks if there is an active synthesize request in progress
 *
 * @return Is there a synethesize currently in progress
 */
bool UWitSocketSubsystem::IsConverseInProgress()
{
	return bConverseInProgress;
}

/**
 * Checks the current status of the WebSocket connection
 *
 * @return Current state of the WebSocket connection
 */
ESocketState UWitSocketSubsystem::GetSocketState()
{
	return SocketStatus;
}

/**
 * Send binary data across an active WebSocket connection
 *
 * @param Data [in] binary data to send over connection
 */
void UWitSocketSubsystem::SendBinaryData(const TArray<uint8>& Data)
{
	const int32 NumBytesToCopy = Data.Num();

	if (NumBytesToCopy <= 0)
	{
		UE_LOG(LogWit, Warning, TEXT("SendBinaryData: No Data to Copy"));
		return;
	}

	if (!Socket->IsConnected())
	{
		UE_LOG(LogWit, Warning, TEXT("SendBinaryData: Not Connected"));
		return;
	}

	if (bWebSocketAuthenticated)
	{
		std::vector<unsigned char> SendData;
		for (int i = 0; i < Data.Num(); i++)
		{
			SendData.push_back(Data[i]);
		}

		const TSharedPtr<FJsonObject> RequestData = MakeShared<FJsonObject>();
		RequestData->SetStringField("client_request_id", RequestId);

		FString StringMessage;
		TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&StringMessage);
		FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);

		std::string Message = Encode(std::string(TCHAR_TO_UTF8(*StringMessage)), SendData);
		UE_LOG(LogWit, Verbose, TEXT("SendBinaryData: Message %d"), Message.length());
		Socket->Send(reinterpret_cast<const uint8_t*>(&Message[0]), Message.length(), true);
	}
	else
	{
		UE_LOG(LogWit, Warning, TEXT("WebSockets: WebSocket not authenticated"));
	}
}

/**
 * Send JSON data across an active WebSocket connection
 *
 * @param Data [in] JSON data to send over connection
 */
void UWitSocketSubsystem::SendJsonData(const ERequestType Type, const TSharedRef<FJsonObject> RequestBody)
{
	if (!Socket->IsConnected())
	{
		UE_LOG(LogWit, Warning, TEXT("SendJsonData: Not Connected"));
		return;
	}

	if (bWebSocketAuthenticated)
	{
		const TSharedPtr<FJsonObject> RequestData = MakeShared<FJsonObject>();
		const TSharedPtr<FJsonObject> RequestSynth = MakeShared<FJsonObject>();
		const FGuid Guid = FGuid();
		switch (Type)
		{
			case ERequestType::Converse:
				RequestSynth->SetObjectField("converse", RequestBody);
				break;
			case ERequestType::Synthesize:
				RequestSynth->SetObjectField("synthesize", RequestBody);
				break;
			default:
				break;
		}
		RequestData->SetObjectField("data", RequestSynth);
		RequestId = Guid.NewGuid().ToString();
		RequestData->SetStringField("client_request_id", RequestId);

		FString StringMessage;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&StringMessage);
		FJsonSerializer::Serialize(RequestData.ToSharedRef(), Writer);

		std::string Message = Encode(std::string(TCHAR_TO_UTF8(*StringMessage)));
		Socket->Send(reinterpret_cast<const uint8_t*>(&Message[0]), Message.length(), true);
	}
	else
	{
		UE_LOG(LogWit, Warning, TEXT("WebSockets: WebSocket not authenticated"));
	}
}

/**
 * Encodes data to send over the WebSocket connection
 *
 * @param JsonData [in] JSON data to send over the WebSocket connection
 * @param BinaryData [in] binary data to send over the WebSocket connection
 */
std::string UWitSocketSubsystem::Encode(
	const std::string& JsonData,
	const std::vector<unsigned char>& BinaryData)
{
	uint8_t Flag = 0;
	if (!BinaryData.empty())
	{
		Flag |= 1 << 0;
		Flag |= 1 << 1;
	}
	else {
		Flag |= 1 << 1;
	}

	size_t JsonStrSize = JsonData.length() * sizeof(char);
	size_t BinaryDataSize = BinaryData.size();

	std::vector<unsigned char> Result;
	Result.resize(1 + 8 + 8 + JsonStrSize + BinaryDataSize);

	Result[0] = Flag;
	std::memcpy(&Result[1], &JsonStrSize, sizeof(JsonStrSize));
	std::memcpy(&Result[9], &BinaryDataSize, sizeof(BinaryDataSize));
	std::memcpy(&Result[17], JsonData.c_str(), JsonStrSize);
	std::memcpy(&Result[17 + JsonStrSize], BinaryData.data(), BinaryDataSize);

	return std::string(reinterpret_cast<const char*>(Result.data()), Result.size());
}

/**
 * Decodes data received over the WebSocket connection
 *
 * @param Message [in] data returned from WebSocket connection
 * @return Data from WebSocket parsed into an EncodedData struct
 */
EncodedData UWitSocketSubsystem::Decode(const std::string Message)
{
	EncodedData Decoded;

	if (Message.size() < 17)
	{
		UE_LOG(LogWit, Warning, TEXT("WebSockets: Message not a valid size"));
		return EncodedData{};
	}

	// Read the length of the JSON data section
	uint32_t JsonDataSize =
		*reinterpret_cast<const uint32_t*>(Message.data() + 1);

	// Read the length of the binary data section
	uint32_t BinaryDataSize =
		*reinterpret_cast<const uint32_t*>(Message.data() + 9);

	// Read the JSON data section
	Decoded.JsonData = std::string(
		reinterpret_cast<const char*>(Message.data() + 17), JsonDataSize);

	// Read the binary data section
	Decoded.BinaryData = std::vector<uint8_t>(
		Message.begin() + 17 + JsonDataSize,
		Message.begin() + 17 + JsonDataSize + BinaryDataSize);

	return Decoded;
}
