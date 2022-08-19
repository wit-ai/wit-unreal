/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Utilities/WitConversionUtilities.h"
#include <limits>

/**
 * Convert a sequence of stereo samples into mono samples
 *
 * @param InSamples [in] the input sequence of stereo samples. The InSamples array must contain 2x NumSamples entries
 * @param NumSamples [in] the input count of stereo samples
 * @param OutSamples [out] the output sequence of corresponding mono samples. The array must be big enough to hold NumSamples entries
 */
void FWitConversionUtilities::ConvertSamplesStereoToMono(const float* InSamples, const int32 NumSamples, float* OutSamples)
{
	for (int32 i = 0; i < NumSamples / 2; ++i)
	{
		const float LeftSample = InSamples[i * 2];
		const float RightSample = InSamples[i * 2 + 1];

		OutSamples[i] = (LeftSample + RightSample) * 0.5f;
	}
}

/**
 * Convert a sequence of floating point samples into 8-bit unsigned samples
 *
 * @param InSamples [in] the input sequence of samples. The InSamples array must contain NumSamples entries
 * @param NumSamples [in] the input count of samples
 * @param OutSamples [out] the output sequence of 8-bit samples. The array must be big enough to hold NumSamples entries
 */
void FWitConversionUtilities::ConvertSamplesFloatTo8Bit(const float* InSamples, const int32 NumSamples, uint8* OutSamples)
{
	constexpr int32 ScaleFactor = std::numeric_limits<int8>::max();

	for (int32 i = 0; i < NumSamples; ++i)
	{
		const int8 ScaledSample = static_cast<int8>(InSamples[i] * ScaleFactor);

		OutSamples[i] = ScaledSample;
	}
}

/**
 * Convert a sequence of floating point samples into 16-bit unsigned samples
 *
 * @param InSamples [in] the input sequence of floating point samples. The InSamples array must contain NumSamples entries
 * @param NumSamples [in] the input count of samples
 * @param OutSamples [out] the output sequence of 16-bit samples. The array must be big enough to hold NumSamples entries
 */
void FWitConversionUtilities::ConvertSamplesFloatTo16Bit(const float* InSamples, const int32 NumSamples, uint8* OutSamples)
{
	constexpr int32 ScaleFactor = std::numeric_limits<int16>::max();

	for (int32 i = 0; i < NumSamples; ++i)
	{
		// There's an assumption here that InSamples are already in the range -1 -> 1. If this assumption is not valid then
		// clamping should be introduced
		
		const int16 ScaledSample = static_cast<int16>(InSamples[i] * ScaleFactor);

		OutSamples[i * 2] = static_cast<int8>(ScaledSample & 0xff);
		OutSamples[i * 2 + 1] = static_cast<int8>((ScaledSample >> 8) & 0xff);
	}
}

/**
 * Convert a sequence of 8-bit samples into floating point samples
 *
 * @param InSamples [in] the input sequence of 8-bit unsigned samples. The InSamples array must contain NumSamples entries
 * @param NumSamples [in] the input count of samples
 * @param OutSamples [out] the output sequence of floating point samples. The array must be big enough to hold NumSamples entries
 */
void FWitConversionUtilities::ConvertSamples8BitToFloat(const uint8* InSamples, const int32 NumSamples, float* OutSamples)
{
	constexpr float ScaleFactor = std::numeric_limits<int8>::max();

	for (int32 i = 0; i < NumSamples; ++i)
	{
		const float ScaledSample = static_cast<float>(InSamples[i]) / ScaleFactor;
		
		OutSamples[i] = ScaledSample < -1.0f ? -1.0f : ScaledSample;
	}
}

/**
 * Convert a sequence of 16-bit samples into floating point samples
 *
 * @param InSamples [in] the input sequence of 16-bit unsigned samples. The InSamples array must contain NumSamples entries
 * @param NumSamples [in] the input count of samples
 * @param OutSamples [out] the output sequence of floating point samples. The array must be big enough to hold NumSamples entries
 */
void FWitConversionUtilities::ConvertSamples16BitToFloat(const uint8* InSamples, const int32 NumSamples, float* OutSamples)
{
	constexpr float ScaleFactor = std::numeric_limits<int16>::max();

	for (int32 i = 0; i < NumSamples; ++i)
	{
		const int16 InSampleValue = InSamples[i * 2] | (InSamples[i * 2 + 1] << 8);
		const float ScaledSample = static_cast<float>(InSampleValue) / ScaleFactor;

		OutSamples[i] = ScaledSample < -1.0f ? -1.0f : ScaledSample;
	}
}

/**
 * Get the maximum amplitude in the given set of samples
 *
 * @param InSamples [in] the input sequence of 16-bit unsigned samples. The InSamples array must contain NumSamples entries
 * @param NumSamples [in] the input count of samples
 * @return the amplitude
 */
float FWitConversionUtilities::CalculateMaximumAmplitude16Bit(const uint8* InSamples, const int32 NumSamples)
{
	int32 MaximumAmplitude = 0;
	
	for (int32 i = 0; i < NumSamples; ++i)
	{
		const int16 InSampleValue = InSamples[i * 2] | (InSamples[i * 2 + 1] << 8);
		const int32 Amplitude = std::abs(InSampleValue);

		if (Amplitude > MaximumAmplitude)
		{
			MaximumAmplitude = Amplitude;
		}
	}

	constexpr float ScaleFactor = std::numeric_limits<int16>::max();
	const float ScaledMaximumAmplitude = static_cast<float>(MaximumAmplitude) / ScaleFactor;

	return ScaledMaximumAmplitude > 1.0f ? 1.0f : ScaledMaximumAmplitude;
}
