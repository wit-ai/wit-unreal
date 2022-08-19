/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"

/**
 * A helper class that contains utilities for converting between different sample sizes and channel counts
 */
class FWitConversionUtilities
{
public:

    /**
     * Convert a sequence of stereo samples into mono samples
     *
     * @param InSamples [in] the input sequence of stereo samples. The InSamples array must contain 2x NumSamples entries
     * @param NumSamples [in] the input count of stereo samples
     * @param OutSamples [out] the output sequence of corresponding mono samples. The array must be big enough to hold NumSamples entries
     */
	static void ConvertSamplesStereoToMono(const float* InSamples, int32 NumSamples, float* OutSamples);

	/**
     * Convert a sequence of floating point samples into 8-bit unsigned samples
     *
     * @param InSamples [in] the input sequence of samples. The InSamples array must contain NumSamples entries
     * @param NumSamples [in] the input count of samples
     * @param OutSamples [out] the output sequence of 8-bit samples. The array must be big enough to hold NumSamples entries
     */
	static void ConvertSamplesFloatTo8Bit(const float* InSamples, int32 NumSamples, uint8* OutSamples);

	/**
     * Convert a sequence of floating point samples into 16-bit unsigned samples
     *
     * @param InSamples [in] the input sequence of floating point samples. The InSamples array must contain NumSamples entries
     * @param NumSamples [in] the input count of samples
     * @param OutSamples [out] the output sequence of 16-bit samples. The array must be big enough to hold NumSamples entries
     */
	static void ConvertSamplesFloatTo16Bit(const float* InSamples, int32 NumSamples, uint8* OutSamples);

	/**
     * Convert a sequence of 8-bit samples into floating point samples
     *
     * @param InSamples [in] the input sequence of 8-bit unsigned samples. The InSamples array must contain NumSamples entries
     * @param NumSamples [in] the input count of samples
     * @param OutSamples [out] the output sequence of floating point samples. The array must be big enough to hold NumSamples entries
     */
	static void ConvertSamples8BitToFloat(const uint8* InSamples, int32 NumSamples, float* OutSamples);

	/**
     * Convert a sequence of 16-bit samples into floating point samples
     *
     * @param InSamples [in] the input sequence of 16-bit unsigned samples. The InSamples array must contain NumSamples entries
     * @param NumSamples [in] the input count of samples
     * @param OutSamples [out] the output sequence of floating point samples. The array must be big enough to hold NumSamples entries
     */
	static void ConvertSamples16BitToFloat(const uint8* InSamples, int32 NumSamples, float* OutSamples);

	/**
	 * Get the largest amplitude in the given set of 16-bit samples
	 *
	 * @param InSamples [in] the input sequence of 16-bit unsigned samples. The InSamples array must contain NumSamples entries
	 * @param NumSamples [in] the input count of samples
	 * @return the amplitude
	 */
	static float CalculateMaximumAmplitude16Bit(const uint8* InSamples, const int32 NumSamples);
};
