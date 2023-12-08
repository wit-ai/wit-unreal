/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Utilities/WitTtsSpeechSplitter.h"
#include "Wit/Utilities/WitLog.h"
#include "Internationalization/Regex.h"

/**
 * Checks whether or not a string needs to be split
 *
 * @param Speech [in] String of speech
 * @param MaxSize [in] The maximum size of text per portion
 */
bool FWitTtsSpeechSplitter::NeedsSplit(const FString Speech, const int32 MaxSize)
{
	return (Speech.Len() > MaxSize);
}

/**
 * Splits a given string into portions
 *
 * @param Speech [in] String of speech
 * @param MaxSize [in] The maximum size of text per portion
 * @return Array of FStrings split portions
 */
TArray<FString> FWitTtsSpeechSplitter::SplitSpeech(const FString Speech, const int32 MaxSize)
{
	TArray<FString> SplitResult;

	const FRegexPattern SentencePattern = FRegexPattern(TEXT("\\.|\\,|\\?|\\;|\\:|\\!"));
	FRegexMatcher Matcher = FRegexMatcher(SentencePattern, Speech);
	int32 LastStart = 0;
	while (Matcher.FindNext())
	{
		int32 Ending = Matcher.GetMatchEnding();

		FString Sentence = Speech.Mid(LastStart, Ending - LastStart);

		if (Sentence.Len() > MaxSize)
		{
			TArray<FString> Words = SplitSentence(Sentence);
			for (int32 i = 0; i < Words.Num(); i++)
			{
				SplitResult.Add(Words[i]);
			}
		}
		else
		{
			SplitResult.Add(Sentence);
		}
		LastStart = Ending;
	}
	FString Sentence = Speech.Mid(LastStart, Speech.Len());

	if (Sentence.Len() > MaxSize)
	{
		TArray<FString> Words = SplitSentence(Sentence);
		for (int32 i = 0; i < Words.Num(); i++)
		{
			SplitResult.Add(Words[i]);
		}
	}
	else
	{
		SplitResult.Add(Sentence);
	}

	return CombineText(SplitResult, MaxSize);
}

/**
 * Splits a given sentence into words
 *
 * @param Sentence [in] A string of words
 * @return Array of FStrings split words
 */
TArray<FString> FWitTtsSpeechSplitter::SplitSentence(const FString Sentence)
{
	TArray<FString> SplitResult;
	const FRegexPattern WordPattern = FRegexPattern(TEXT("\\s"));

	FRegexMatcher Matcher = FRegexMatcher(WordPattern, Sentence);
	int32 LastStart = 0;
	while (Matcher.FindNext())
	{
		int32 Ending = Matcher.GetMatchEnding();

		FString Word = Sentence.Mid(LastStart, Ending - LastStart);

		SplitResult.Add(Word);
		LastStart = Ending;
	}
	FString Word = Sentence.Mid(LastStart, Sentence.Len());
	SplitResult.Add(Word);

	return SplitResult;
}

/**
 * Combines portions of text together that are no larger than a given size
 *
 * @param Portions [in] a series of fragements
 * @param MaxSize [in] The maximum size of text per portion
 * @return Array of FStrings combined portions
 */
TArray<FString> FWitTtsSpeechSplitter::CombineText(const TArray<FString> Portions, const int32 MaxSize)
{
	TArray<FString> CombinedText;
	FString CurrentText;
	for (FString Portion : Portions)
	{
		FString TempText = CurrentText + Portion;
		if (TempText.Len() > MaxSize)
		{
			CombinedText.Add(CurrentText);
			CurrentText = Portion;
		}
		else
		{
			CurrentText = TempText;
		}
	}
	CombinedText.Add(CurrentText);
	return CombinedText;
}
