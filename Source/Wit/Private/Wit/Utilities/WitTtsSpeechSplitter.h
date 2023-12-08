/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"

 /**
  * A helper class that contains utilities for splitting strings of speech
  */
class FWitTtsSpeechSplitter
{
public:
	/**
	 * Checks whether or not a string needs to be split
	 * 
	 * @param Speech [in] String of speech
	 * @param MaxSize [in] The maximum size of text per portion
	 */
	static bool NeedsSplit(const FString Speech, const int32 MaxSize);

	/**
	 * Splits a given string into portions
	 *
	 * @param Speech [in] String of speech
	 * @param MaxSize [in] The maximum size of text per portion
	 * @return Array of FStrings split portions 
	 */
	static TArray<FString> SplitSpeech(const FString Speech, const int32 MaxSize);

protected:
	/**
	 * Splits a given sentence into words
	 *
	 * @param Sentence [in] A string of words
	 * @return Array of FStrings split words
	 */
	static TArray<FString> SplitSentence(const FString Sentence);

	/**
	 * Combines portions of text together that are no larger than a given size
	 *
	 * @param Portions [in] a series of fragements
	 * @param MaxSize [in] The maximum size of text per portion
	 * @return Array of FStrings combined portions
	 */
	static TArray<FString> CombineText(const TArray<FString> Portions, const int32 MaxSize);
};
