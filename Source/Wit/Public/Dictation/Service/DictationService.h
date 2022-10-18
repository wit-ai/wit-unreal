/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/IDictationServiceBase.h"
#include "Dictation/Events/DictationEvents.h"
#include "DictationService.generated.h"

/**
 * Base class implementation of a dictation service
 */
UCLASS(ClassGroup=(Meta), Abstract)
class WIT_API UDictationService : public UActorComponent, public IDictationServiceBase
{
	GENERATED_BODY()

public:
	
	UDictationService() : Super() {};

	/**
	 * Set the configuration to use for dictation
	 */
	void SetConfiguration(const FDictationConfiguration* ConfigurationToUse) { Configuration = ConfigurationToUse; }

	/**
	 * Set the events to use with this service
	 */
	void SetEvents(UDictationEvents* EventsToUse) { Events = EventsToUse; }
	
	/**
	 * IDictationService overrides
	 */
	virtual bool ActivateDictation() override { return false; }
	virtual bool ActivateDictationWithRequestOptions(const FString& RequestOptions)override { return false; }
	virtual bool ActivateDictationImmediately() override { return false; }
	virtual bool DeactivateDictation() override { return false; }
	virtual bool IsDictationActive() const override { return false; }
	virtual bool IsRequestInProgress() const override { return false; }

protected:

	/** Configuration to use with the dictation service */
	const FDictationConfiguration* Configuration{};

	/**
	 * The events that this service should use in callbacks
	 */
	UPROPERTY(Transient)
	UDictationEvents* Events{};

};
