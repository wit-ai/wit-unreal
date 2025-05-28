/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Voice/Service/VoiceService.h"
#include "Wit/Request/WitRequestTypes.h"
#include "Wit/TTS/WitTtsService.h"
#include "WitVoiceService.generated.h"

#ifdef CPP_PLUGIN
THIRD_PARTY_INCLUDES_START
#include "VoiceSDK/v2/interfaces/io/IAudioStreamInputProvider.hpp"

using namespace meta::voicesdk::v2;

THIRD_PARTY_INCLUDES_END
#endif

class FJsonObject;
/**
 * Component that encapsulates the Wit Voice Command API. Provides functionality for making speech and message requests
 * to Wit.ai to interpret and extract meaning. To use it simply attach the UWitVoiceService component in the hierarchy of any Actor
 */
UCLASS(ClassGroup=(Meta), meta=(BlueprintSpawnableComponent))
class WIT_API UWitVoiceService final : public UVoiceService
{
	GENERATED_BODY()

public:
	
	UWitVoiceService();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * VoiceService overrides
	 */
	virtual void SetConfiguration(UWitAppConfigurationAsset* ConfigurationToUse, bool bUseWebSocketToUse) override;

	/**
	 * IVoiceService overrides
	 */
	virtual bool ActivateVoiceInput() override;
	virtual bool ActivateVoiceInputWithRequestOptions(const FString& RequestOptions) override;
	virtual bool ActivateVoiceInputImmediately() override;
	virtual bool ActivateVoiceInputImmediatelyWithRequestOptions(const FString& RequestOptions) override;
	virtual bool DeactivateVoiceInput() override;
	virtual bool DeactivateAndAbortRequest() override;
	virtual bool IsVoiceInputActive() const override;
	virtual float GetVoiceInputVolume() const override;
	virtual bool IsVoiceStreamingActive() const override;
	virtual bool IsRequestInProgress() const override;
	virtual void SendTranscription(const FString& Text) override;
	virtual void SendTranscriptionWithRequestOptions(const FString& Text, const FString& RequestOptions) override;
	virtual void AcceptPartialResponseAndCancelRequest(const FWitResponse& Response) override;

protected:
	
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	
private:

#if WITH_EDITOR

	/** Write the captured voice input to a wav file */
	static void WriteRawPCMDataToWavFile(const uint8* RawPCMData, int32 RawPCMDataSize, int32 NumChannels, int32 SampleRate);

#endif

	void OnSocketStateChange();

	/** Start the Wit speech request */
	void BeginStreamRequest();

	/** Do the actual bulk of the deactivation */
	bool DoDeactivateVoiceInput();

	/** Called when a Wit speech request is in progress to retrieve any changes to the response payload */
	void OnSpeechRequestProgress(const TArray<uint8>& PartialBinaryResponse, const TSharedPtr<FJsonObject> PartialJsonResponse) const;

	/** Called when received a Wit partial response */
	void OnPartialResponse(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse) const;
	
	/** Called when a Wit message(Transcription) request is fully completed to process the response payload */
	void OnMessageRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse);
	
	/** Called when a Wit speech request is fully completed to process the response payload */
	void OnSpeechRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse);
	
	/** Called when a Wit voice request is fully completed to process the response payload */
	void OnRequestComplete(const TArray<uint8>& BinaryResponse, const TSharedPtr<FJsonObject> JsonResponse, const bool bIsResponseRestNeeded);
	
	/** Called when a Wit voice request is fully completed to process the response payload */
	void OnRequestComplete(const FWitResponse& Response) const;

	/** Called when a Wit request errors */
	void OnWitRequestError(const FString& ErrorMessage, const FString& HumanReadableMessage) const;

	/** The audio format that will be passed to Wit when making /speech requests. Currently only Raw is supported */
	const EWitRequestFormat Format{EWitRequestFormat::Raw};

	/** The audio encoding that will be passed to Wit when making /speech requests. Currently only SignedInteger is supported */
	const EWitRequestEncoding Encoding{EWitRequestEncoding::SignedInteger};

	/** The sample size that will be passed to Wit when making /speech requests. Currently only 16-bit word is supported */
	const EWitRequestSampleSize SampleSize{EWitRequestSampleSize::Word};

	/** Used to track when voice input is active on this component */
	bool bIsVoiceInputActive{false};

	/** Used to track when voice streaming is active on this component */
	bool bIsVoiceStreamingActive{false};

	/** Used to track how long since we received voice data when capturing */
	float LastVoiceTime{0.0f};

	/** Used to track how long since we activated voice input when capturing */
	float LastActivateTime{0.0f};

	/** Used to track how long since we reached wake volume when capturing */
	float LastWakeTime{0.0f};


#ifdef CPP_PLUGIN
#if PLATFORM_ANDROID
	std::shared_ptr<IAudioStreamInputProvider> StreamInputProvider;
#endif
#endif
};
