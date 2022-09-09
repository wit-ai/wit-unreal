/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Composer/Configuration/ComposerConfiguration.h"
#include "Composer/Data/ComposerContextMap.h"
#include "Composer/Events/ComposerEvents.h"
#include "Composer/Handlers/Action/ComposerActionHandler.h"
#include "Composer/Handlers/Speech/ComposerSpeechHandler.h"
#include "Voice/Experience/VoiceExperience.h"
#include "WitComposerService.generated.h"

/**
 * Component that encapsulates the Wit Composer API. Provides functionality for making complex interactive experiences
 */
UCLASS(ClassGroup=(Meta), meta=(BlueprintSpawnableComponent))
class WIT_API UWitComposerService final : public UActorComponent
{
	GENERATED_BODY()

public:

	UWitComposerService();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	/**
	 * Set the configuration to use for composer
	 */
	void SetConfiguration(const FComposerConfiguration* ConfigurationToUse);

	/**
	 * Set the handlers to use
	 */
	void SetHandlers(UComposerEvents* EventHandlerToUse, UComposerActionHandler* ActionHandlerToUse, UComposerSpeechHandler* SpeechHandlerToUse);

	/**
	 * Start a new composer session
	 *
	 * @param NewSessionId [in] the session id to use. If empty then the default session id will be used
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|Session")
	void StartSession(FString NewSessionId);

	/**
	 * End the current composer session
	 */
	UFUNCTION(BlueprintCallable, Category = "Composer|Session")
	void EndSession();
	
	/**
	 * Creates a default session id
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Composer|Session")
	static FString GetDefaultSessionId();

	/**
	 * Update the context map
	 *
	 * @param NewContextMap [in] the context map
	 */
	UFUNCTION(BlueprintSetter, Category = "Composer|ContextMap")
	void SetContextMap(UComposerContextMap* NewContextMap);

	/**
	 * Access the the context map
	 *
	 * @return the current context map
	 */
	UFUNCTION(BlueprintGetter, Category = "Composer|ContextMap")
	UComposerContextMap* GetContextMap() const { return CurrentContextMap; }

protected:

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

private:

	/** Callback to redirect the speech and message endpoints to the composer equivalents */
	void OnComposerRequestCustomize(FWitRequestConfiguration& RequestConfiguration);

	/** Callback when we receive a wit response */
	void OnComposerResponse(const TArray<uint8>& BinaryResponse, TSharedPtr<FJsonObject> JsonResponse);

	/** Callback when a wit error occurs */
	void OnComposerError(const FString& ErrorMessage, const FString& HumanReadableMessage);

	/** Internal function for updating the context map from a Json object */
	void SetContextMapInternal(TSharedPtr<FJsonObject> ContextMapJsonObject);

	/** Speak a given phrase */
	void DoSpeakPhrase(const FString& Phrase) const;

	/** Perform a given action */
	void DoPerformAction(const FString& Action) const;

	/** Continue on after an action/speech */
	void DoContinue();
	
	/** Are we allowed to continue after an action/speech? */
	bool CanContinue() const;

	/** Unique session id for the current session */
	FString SessionId{};

	/** When the current session started */
	FDateTime SessionStart{};

	/** Composer response data from the most recent request */
	FWitComposerResponse ComposerResponse{};

	/** Configuration to use with the voice service */
	const FComposerConfiguration* Configuration{};

	/** Set when we are waiting to continue after speech or an action */
	bool bIsWaitingToContinue{false};

	/** Count down delay timer after continue is allowed */
	float ContinueDelayTimer{0.0f};

	/**
	 * The voice experience that composer will use
	 */
	UPROPERTY(Transient)
	AVoiceExperience* VoiceExperience{};

	/**
	 * The event handler that this service should use in callbacks
	 */
	UPROPERTY(Transient)
	UComposerEvents* EventHandler{};

	/**
	 * Action handler to handle any actions from a composer graph
	 */
	UPROPERTY(Transient)
	UComposerActionHandler* ActionHandler{};

	/**
	 * Speech handler to handle any speech from a composer graph
	 */
	UPROPERTY(Transient)
	UComposerSpeechHandler* SpeechHandler{};

	/**
	 * Current context map being used with the composer service
	 */
	UPROPERTY(Transient, BlueprintGetter=GetContextMap, BlueprintSetter=SetContextMap, Category = "Composer|ContextMap")
	UComposerContextMap* CurrentContextMap{};
	
};
