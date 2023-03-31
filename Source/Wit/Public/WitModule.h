/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"

class FHttpManager;

/**
 * The module for the UE4 plugin implementation of the Wit API
 */
class FWitModule final : public IModuleInterface
{
public:

	/**
	 * Voice SDK name
	 */
	static const FString Name;

	/**
	 * Wit API version
	 */
	const FString& SdkVersion = Version;

	/**
	 * IModuleInterface implementation
	 */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return singleton instance, loading the module on demand if needed
	 */
	WIT_API static FWitModule& Get();

	/**
	 * Only meant to be used by Http request/response implementations
	 *
	 * @return http request manager used by the module
	 */
	FHttpManager& GetHttpManager() const
	{
		check(HttpManager != nullptr);
		return *HttpManager;
	}

private:

	/** Keeps track of Http requests while they are being processed */
	FHttpManager* HttpManager{nullptr};

	/** Wit API version */
	FString Version;

	/** Singleton for the module while loaded and available */
	static FWitModule* Singleton;
};
