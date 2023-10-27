/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitRequestBuilder.h"
#include "Wit/Request/WitRequestSubsystem.h"

/** Default wit.ai URL */
const FString FWitRequestBuilder::UrlDefault = TEXT("https://api.wit.ai");

/** Supported wit.ai endpoints */
const FString FWitRequestBuilder::EndpointSpeech = TEXT("speech");
const FString FWitRequestBuilder::EndpointMessage = TEXT("message");
const FString FWitRequestBuilder::EndpointSynthesize = TEXT("synthesize");
const FString FWitRequestBuilder::EndpointVoices = TEXT("voices");
const FString FWitRequestBuilder::EndpointConverse = TEXT("converse");
const FString FWitRequestBuilder::EndpointEvent = TEXT("event");
const FString FWitRequestBuilder::EndpointGetApps = TEXT("apps");
const FString FWitRequestBuilder::EndpointGetEntities = TEXT("entities");
const FString FWitRequestBuilder::EndpointGetIntents = TEXT("intents");
const FString FWitRequestBuilder::EndpointGetTraits = TEXT("traits");
const FString FWitRequestBuilder::EndpointClientToken = TEXT("apps/{0}/client_tokens");
const FString FWitRequestBuilder::EndpointDictation = TEXT("dictation");

/** Supported wit.ai parameters */
const FString FWitRequestBuilder::ParameterTextKey = TEXT("&q=");
const FString FWitRequestBuilder::ParameterSessionId = TEXT("&session_id=");
const FString FWitRequestBuilder::ParameterContextMap = TEXT("&context_map=");
const FString FWitRequestBuilder::ParameterOffset = TEXT("&offset=");
const FString FWitRequestBuilder::ParameterLimit = TEXT("&limit=");

/** Supported wit.ai content formats */
const FString FWitRequestBuilder::FormatKey = TEXT("");
const FString FWitRequestBuilder::FormatValueRaw = TEXT("audio/raw");
const FString FWitRequestBuilder::FormatValueWav = TEXT("audio/wav");
const FString FWitRequestBuilder::FormatValueJson = TEXT("application/json");

/** Supported wit.ai audio encodings */
const FString FWitRequestBuilder::EncodingKey = TEXT("encoding=");
const FString FWitRequestBuilder::EncodingValueFloatingPoint = TEXT("floating-point");
const FString FWitRequestBuilder::EncodingValueSignedInteger = TEXT("signed-integer");
const FString FWitRequestBuilder::EncodingValueUnsignedInteger = TEXT("unsigned-integer");

/** Supported wit.ai audio sample sizes */
const FString FWitRequestBuilder::SampleSizeKey = TEXT("bits=");
const FString FWitRequestBuilder::SampleSizeValueByte = TEXT("8");
const FString FWitRequestBuilder::SampleSizeValueWord = TEXT("16");
const FString FWitRequestBuilder::SampleSizeValueDword = TEXT("32");

/** Supported wit.ai audio sample rates */
const FString FWitRequestBuilder::RateKey = TEXT("rate=");

/** Supported wit.ai audio endianness */
const FString FWitRequestBuilder::EndianKey = TEXT("endian=");
const FString FWitRequestBuilder::EndianValueLittle = TEXT("little");
const FString FWitRequestBuilder::EndianValueBig = TEXT("big");

/**
 * Setup a configuration with required information
 *
 * @param Configuration the request configuration to fill in
 * @param Endpoint the endpoint we want to use
 * @param AuthToken the server auth token for our application (this can be retrieved from the setup in Wit.ai)
 * @param Version the version string we want to use. This can be left empty in which case it will use the latest version
 * @param CustomUrl the custom base URL to use. Normally this should be left empty to use the default Wit.ai URL
 */
void FWitRequestBuilder::SetRequestConfigurationWithDefaults(FWitRequestConfiguration& Configuration, const EWitRequestEndpoint Endpoint, const FString& AuthToken, const FString& Version, const FString& CustomUrl)
{
	if (!CustomUrl.IsEmpty())
	{
		Configuration.BaseUrl = CustomUrl;
	}
	else
	{
		Configuration.BaseUrl = UrlDefault;
	}

	Configuration.Version = Version;
	Configuration.AuthToken = AuthToken;
	Configuration.Endpoint = GetEndpointString(Endpoint);
	Configuration.Verb = GetVerbString(Endpoint);
	Configuration.bShouldUseChunkedTransfer = Endpoint == EWitRequestEndpoint::Speech || Endpoint == EWitRequestEndpoint::Converse || Endpoint == EWitRequestEndpoint::Dictation;
}

/**
 * Add a text URL parameter. This is required when using the /message endpoint
 *
 * @param Configuration the request configuration to fill in
 * @param ParameterKey the parameter key
 * @param ParameterValue the URL encoded text to use as a parameter
 */
void FWitRequestBuilder::AddParameter(FWitRequestConfiguration& Configuration, const EWitParameter ParameterKey, const FString& ParameterValue)
{
	const FString& ParameterKeyString(GetParameterKeyString(ParameterKey));

	check(!Configuration.Parameters.Contains(ParameterKeyString));

	Configuration.Parameters.Emplace(ParameterKeyString, ParameterValue);
}

/**
 * Add a Mime expect type. This is required for the /synthesize endpoint
 *
 * @param Configuration [in,out] the request configuration to fill in
 * @param Format [in] the desired accept format
 */
void FWitRequestBuilder::AddFormatAccept(FWitRequestConfiguration& Configuration, const EWitRequestAudioFormat Format)
{
	Configuration.Accept = GetFormatAudioString(Format);
}

/**
 * Add an audio format content type. This is required when using the /speech endpoint
 *
 * @param Configuration the request configuration to fill in
 * @param Format the desired content format
 */
void FWitRequestBuilder::AddFormatContentType(FWitRequestConfiguration& Configuration, const EWitRequestFormat Format)
{
	check(!Configuration.ContentTypes.Contains(FormatKey));

	Configuration.ContentTypes.Emplace(FormatKey, GetFormatString(Format));
}

/**
 * Add an audio encoding type. This is required when using the /speech endpoint
 *
 * @param Configuration the request configuration to fill in
 * @param Encoding the desired audio encoding
 */
void FWitRequestBuilder::AddEncodingContentType(FWitRequestConfiguration& Configuration, const EWitRequestEncoding Encoding)
{
	check(!Configuration.ContentTypes.Contains(EncodingKey));

	Configuration.ContentTypes.Emplace(EncodingKey, GetEncodingString(Encoding));
}

/**
 * Add an audio sample size type. This is required when using the /speech endpoint
 *
 * @param Configuration the request configuration to fill in
 * @param SampleSize the desired audio sample size
 */
void FWitRequestBuilder::AddSampleSizeContentType(FWitRequestConfiguration& Configuration, const EWitRequestSampleSize SampleSize)
{
	check(!Configuration.ContentTypes.Contains(SampleSizeKey));

	Configuration.ContentTypes.Emplace(SampleSizeKey, GetSampleSizeString(SampleSize));
}

/**
 * Add an audio rate type. This is required when using the /speech endpoint
 *
 * @param Configuration the request configuration to fill in
 * @param Rate the desired audio sample rate. Not all rates are necessarily valid and only rates > 0 will be accepted
 */
void FWitRequestBuilder::AddRateContentType(FWitRequestConfiguration& Configuration, const int32 Rate)
{
	check(Rate > 0);
	check(!Configuration.ContentTypes.Contains(RateKey));

	Configuration.ContentTypes.Emplace(RateKey, FString::FromInt(Rate));
}

/**
 * Add an audio endianness type. This is required when using the /speech endpoint
 *
 * @param Configuration the request configuration to fill in
 * @param Endian the desired audio endianness
 */
void FWitRequestBuilder::AddEndianContentType(FWitRequestConfiguration& Configuration, const EWitRequestEndian Endian)
{
	check(!Configuration.ContentTypes.Contains(EndianKey));

	Configuration.ContentTypes.Emplace(EndianKey, GetEndianString(Endian));
}

/**
 * Converts an endpoint into its final string representation
 *
 * @param Endpoint the endpoint
 * @return the string representation of the endpoint
 */
const FString& FWitRequestBuilder::GetEndpointString(const EWitRequestEndpoint Endpoint)
{
	switch (Endpoint)
	{
	case EWitRequestEndpoint::Speech:
		{
			return EndpointSpeech;
		}
	case EWitRequestEndpoint::Message:
		{
			return EndpointMessage;
		}
	case EWitRequestEndpoint::Synthesize:
		{
			return EndpointSynthesize;
		}
	case EWitRequestEndpoint::GetVoices:
		{
			return EndpointVoices;
		}
	case EWitRequestEndpoint::Converse:
		{
			return EndpointConverse;
		}
	case EWitRequestEndpoint::Event:
		{
			return EndpointEvent;
		}
	case EWitRequestEndpoint::GetApps:
		{
			return EndpointGetApps;
		}
	case EWitRequestEndpoint::GetEntities:
		{
			return EndpointGetEntities;
		}
	case EWitRequestEndpoint::GetIntents:
		{
			return EndpointGetIntents;
		}
	case EWitRequestEndpoint::GetTraits:
		{
			return EndpointGetTraits;
		}
	case EWitRequestEndpoint::ClientToken:
		{
			return EndpointClientToken;
		}
	case EWitRequestEndpoint::Dictation:
		{
			return EndpointDictation;
		}
	default:
		{
			check(0);
			return EndpointSpeech;
		}
	}
}

/**
 * Converts an endpoint into its verb string representation
 *
 * @param Endpoint the endpoint
 * @return the string representation of the verb
 */
FString FWitRequestBuilder::GetVerbString(const EWitRequestEndpoint Endpoint)
{
	switch (Endpoint)
	{
	case EWitRequestEndpoint::Speech:
	case EWitRequestEndpoint::Synthesize:
	case EWitRequestEndpoint::Converse:
	case EWitRequestEndpoint::Event:
	case EWitRequestEndpoint::ClientToken:
	case EWitRequestEndpoint::Dictation:
		{
			return TEXT("POST");
		}
	default:
		{
			break;
		}
	}

	return TEXT("GET");
}

/**
 * Converts a parameter key to its string representation
 *
 * @param ParameterKey the parameter key
 * @return the string representation of the parameter key
 */
const FString& FWitRequestBuilder::GetParameterKeyString(const EWitParameter ParameterKey)
{
	switch (ParameterKey)
	{
	case EWitParameter::Text:
		{
			return ParameterTextKey;
		}
	case EWitParameter::SessionId:
		{
			return ParameterSessionId;
		}
	case EWitParameter::ContextMap:
		{
			return ParameterContextMap;
		}
	case EWitParameter::Offset:
		{
			return ParameterOffset;
		}
	case EWitParameter::Limit:
		{
			return ParameterLimit;
		}
	default:
		{
			check(0);
			return ParameterTextKey;
		}
	}
}

/**
 * Converts an audio format into its final string representation
 *
 * @param Format the audio format
 * @return the string representation of the audio format
 */
const FString& FWitRequestBuilder::GetFormatAudioString(const EWitRequestAudioFormat Format)
{
	switch (Format)
	{
	case EWitRequestAudioFormat::Pcm:
		{
			return FormatValueRaw;
		}
	case EWitRequestAudioFormat::Wav:
		{
			return FormatValueWav;
		}
	default:
		{
			check(0);
			return FormatValueWav;
		}
	}
}

/**
 * Converts a request header value into its final string representation
 *
 * @param Format the request header value
 * @return the string representation of the request header value
 */
const FString& FWitRequestBuilder::GetFormatString(const EWitRequestFormat Format)
{
	switch (Format)
	{
	case EWitRequestFormat::Raw:
		{
			return FormatValueRaw;
		}
	case EWitRequestFormat::Wav:
		{
			return FormatValueWav;
		}
	case EWitRequestFormat::Json:
		{
			return FormatValueJson;
		}
	default:
		{
			check(0);
			return FormatValueJson;
		}
	}
}

/**
 * Converts an audio encoding into its final string representation
 *
 * @param Encoding the audio encoding
 * @return the string representation of the audio encoding
 */
const FString& FWitRequestBuilder::GetEncodingString(const EWitRequestEncoding Encoding)
{
	switch (Encoding)
	{
	case EWitRequestEncoding::FloatingPoint:
		{
			return EncodingValueFloatingPoint;
		}
	case EWitRequestEncoding::SignedInteger:
		{
			return EncodingValueSignedInteger;
		}
	case EWitRequestEncoding::UnsignedInteger:
		{
			return EncodingValueUnsignedInteger;
		}
	default:
		{
			check(0);
			return EncodingValueSignedInteger;
		}
	}
}

/**
 * Converts an audio sample size into its final string representation
 *
 * @param SampleSize the audio sample size
 * @return the string representation of the audio sample size
 */
const FString& FWitRequestBuilder::GetSampleSizeString(const EWitRequestSampleSize SampleSize)
{
	switch (SampleSize)
	{
	case EWitRequestSampleSize::Byte:
		{
			return SampleSizeValueByte;
		}
	case EWitRequestSampleSize::Word:
		{
			return SampleSizeValueWord;
		}
	case EWitRequestSampleSize::DoubleWord:
		{
			return SampleSizeValueDword;
		}
	default:
		{
			check(0);
			return SampleSizeValueWord;
		}
	}
}

/**
 * Converts an audio endianness into its final string representation
 *
 * @param Endian the audio endianness
 * @return the string representation of the audio endianness
 */
const FString& FWitRequestBuilder::GetEndianString(const EWitRequestEndian Endian)
{
	switch (Endian)
	{
	case EWitRequestEndian::Little:
		{
			return EndianValueLittle;
		}
	case EWitRequestEndian::Big:
		{
			return EndianValueBig;
		}
	default:
		{
			check(0);
			return EndianValueLittle;
		}
	}
}
