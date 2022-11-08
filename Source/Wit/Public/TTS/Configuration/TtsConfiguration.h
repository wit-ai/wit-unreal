/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "TtsConfiguration.generated.h"

UENUM()
enum class ETtsStorageCacheLocation : uint8
{
	Default,	// Use the default cache location specified in the storage cache
	None,		// Do not cache
	Content,	// Use the content folder. Use this for clips you want to to package into your app
	Persistent,	// Use the persistent app folder. Use this for clips you want to persist across runs of your app
	Temporary	// Use the temporary app folder. Use this for clips you don't mind being thrown away across runs of your app
};

/**
 * Voice configuration for /synthesize endpoint of Wit.ai.
 */
USTRUCT(BlueprintType)
struct WIT_API FTtsConfiguration
{
	GENERATED_BODY()

	/** The text we want to convert */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TTS")
	FString Text{};
	
	/** The voice name that should be used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TTS")
	FString Voice{TEXT("Charlie")};

	/** The voice style that should be used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TTS")
	FString Style{};

	/** How fast the voice will be spoken with 50 being slow, 200 fast. Default is 100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 50, ClampMax = 200), Category="TTS")
	int32 Speed{100};

	/** Pitch of the voice. This can range between 25% and 400% */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 25, ClampMax = 400), Category="TTS")
	int32 Pitch{100};

	/** Gain of the voice. This can range between 0% and 100% */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin = 0, ClampMax = 100), Category="TTS")
	int32 Gain{50};

	/** If set to default this uses the globally specified cache location. Any other value acts as an override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TTS")
	ETtsStorageCacheLocation StorageCacheLocation{ETtsStorageCacheLocation::Default};
};
