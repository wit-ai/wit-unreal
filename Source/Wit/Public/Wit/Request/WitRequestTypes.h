/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "WitRequestTypes.generated.h"

/**
 * A list of the available endpoints for Wit.ai.
 */
UENUM()
enum class EWitRequestEndpoint : uint8
{
	Speech,
	Message,
	Synthesize,
	GetVoices,
	Converse,
	Event,
	GetApps,
	GetEntities,
	GetIntents,
	GetTraits,
	ClientToken,
	Dictation
};

/**
 * A list of the available parameters for Wit.ai
 */
UENUM()
enum class EWitParameter : uint8
{
	Text,
	SessionId,
	ContextMap,
	Offset,
	Limit
};

/**
 * A list of the available audio formats for the /speech endpoint of Wit.ai.
 */
UENUM()
enum class EWitRequestAudioFormat : uint8
{
	Pcm,
	Wav
};

/**
 * A list of the available request header values for the /speech endpoint of Wit.ai.
 */
UENUM()
enum class EWitRequestFormat : uint8
{
	Raw,
	Wav,
	Json
};

/**
 * A list of the available audio encodings for the /speech endpoint of Wit.ai.
 */
UENUM()
enum class EWitRequestEncoding : uint8
{
	SignedInteger,
	FloatingPoint,
	UnsignedInteger,
};

/**
 * A list of the available audio sample sizes for the /speech endpoint of Wit.ai.
 */
UENUM()
enum class EWitRequestSampleSize : uint8
{
	Byte,
	Word,
	DoubleWord
};

/**
 * A list of the available audio endianness for the /speech endpoint of Wit.ai.
 */
UENUM()
enum class EWitRequestEndian : uint8
{
	Little,
	Big
};
