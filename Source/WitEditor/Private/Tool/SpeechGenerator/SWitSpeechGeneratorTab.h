/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Wit/Request/WitResponse.h"
#include "TTS/Configuration/TtsVoicePresetAsset.h"
#include "TTS/Experience/TtsExperience.h"
#include "SWitSpeechGeneratorTab.generated.h"

class AVoiceExperience;

/**
 * Single text item we want to convert to speech
 */
USTRUCT(BlueprintType)
struct FWitTextItem
{
	GENERATED_BODY()

	UPROPERTY(Transient, EditAnywhere, Category = "Text Item")
	FString Text{};

	UPROPERTY(Transient, EditAnywhere, Category = "Text Item")
	UTtsVoicePresetAsset* VoicePreset{};

	UPROPERTY(Transient, VisibleAnywhere, Category = "Text Item")
	FString ClipId{};
	
};

/**
 * Collection of text items that we want to convert to speech
 */
UCLASS(BlueprintType)
class UWitTextCollection final : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(Transient, EditAnywhere, Category = "Text Collection")
	TArray<FWitTextItem> Items{};

	/** Callback to receive the converted sound wave files */
	UFUNCTION()
	void OnSynthesizeRawResponse(const FString& ClipId, const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings);

	/** The current text entered by the user as an utterance */
	FText ContentOutputLocation{};

	/** Is a convert in progress? */
	bool bIsConvertInProgress{false};

	/** Index of current item we are converting */
	int32 CurrentConvertIndex{0};
	
	/** Convert a single text item */
	void ConvertTextItem(FWitTextItem& ItemToConvert);
	
};

/**
 * Slate widget to represent the Understanding viewer. The Understanding viewer is a simple tool to be able to send
 * requests to Wit.ai and see the results in the editor
 */
class SWitSpeechGeneratorTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SWitSpeechGeneratorTab) {}
	SLATE_END_ARGS()

	/** Define the slate layout for the widget */
	void Construct(const FArguments& InArgs);

	/** Gets the currently selected voice experience or null if there is none */ 
	static ATtsExperience* GetSelectedTtsExperience();

protected:

	/** Callbacks used by the editable text box of the utterance text to track when the input text changes */
	void OnOutputLocationTextChanged(const FText& InText);
	void OnOutputLocationTextCommitted(const FText& InText, ETextCommit::Type);

	/** Callbacks used by the send button */
	FReply OnConvertButtonClicked();
	bool IsConvertButtonEnabled() const;

	/** Callbacks used to determine visibility of the tool usage messages */
	EVisibility GetSelectMessageVisibility() const;
	EVisibility GetWaitMessageVisibility() const;
	EVisibility GetResultVisibility() const;

	/** Details widget that will display the response that gets returned from Wit.ai */
	TSharedPtr<IDetailsView> DetailsWidget{};

	/** A UObject wrapper for the response structure so that we can display it in the details widget */
	UWitTextCollection* TextCollection{};
	
};
