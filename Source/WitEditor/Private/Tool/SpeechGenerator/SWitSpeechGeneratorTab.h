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

	/** The text to convert to speech */
	UPROPERTY(EditAnywhere, Category = "Text Item")
	FString Text{};

	/** The voice preset to use during the conversion */
	UPROPERTY(EditAnywhere, Category = "Text Item")
	UTtsVoicePresetAsset* VoicePreset{};

	/** Read only value that shows the file name the clip is written to */
	UPROPERTY(VisibleAnywhere, Category = "Text Item")
	FString ClipId{};
	
};

/**
 * TextCollection of text items that we want to convert to speech
 */
UCLASS(BlueprintType)
class UWitTextCollectionAsset final : public UDataAsset
{
	GENERATED_BODY()

public:

	/** The content folder to output the converted audio files to. This is relative to the base content folder */
	UPROPERTY(EditAnywhere, Category = "Output Location")
	FString ContentFolder{TEXT("Wit/Cache")};

	/** The text items we want to batch convert */
	UPROPERTY(EditAnywhere, Category = "Text To Convert")
	TArray<FWitTextItem> Items{};

};

/**
 * The currently edited text collection
 */
UCLASS(BlueprintType)
class UWitEditedTextCollection final : public UObject
{
	GENERATED_BODY()

public:
	
	/** The currently selected text collection we are viewing */
	UPROPERTY(Transient, EditAnywhere, Category = "Selected")
	UWitTextCollectionAsset* TextCollection{};
	
	/** Callback to receive the converted sound wave files */
	UFUNCTION()
	void OnSynthesizeRawResponse(const FString& ClipId, const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings);

	/** Callback when an error occurs */
	UFUNCTION()
	void OnSynthesizeError(const FString& ErrorMessage, const FString& HumanReadableMessage);

	/** Is a convert in progress? */
	bool bIsConvertInProgress{false};

	/** Index of current item we are converting */
	int32 CurrentConvertIndex{0};

	/** The details widget that will display this UObject */
	TSharedPtr<IDetailsView> DetailsContentWidget{};

	/** Convert a single text item */
	void ConvertTextItem(FWitTextItem& ItemToConvert);

	/** Find the next valid item to convert */
	int32 FindNextItemToConvert();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (DetailsContentWidget != nullptr)
		{
			DetailsContentWidget->SetObject(TextCollection);
		}
	}		
};

/**
 * Slate widget to represent the Speech generator. The Speech generator is a simple tool to be able to batch convert text to speech
 * using wit.ai's TTS
 */
class SWitSpeechGeneratorTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SWitSpeechGeneratorTab) {}
	SLATE_END_ARGS()

	/** Define the slate layout for the widget */
	void Construct(const FArguments& InArgs);

	/** Gets the currently selected TTS experience or null if there is none */ 
	static ATtsExperience* GetSelectedTtsExperience();

protected:

	/** Callbacks used by the editable text box of the new collection text to track when the input text changes */
	void OnNewCollectionTextChanged(const FText& InText);
	void OnNewCollectionTextCommitted(const FText& InText, ETextCommit::Type);

	/** Callbacks used by the convert button */
	FReply OnConvertButtonClicked();
	bool IsConvertButtonEnabled() const;

	/** Callback when the new button is clicked */
	FReply OnNewButtonClicked();
	bool IsNewButtonEnabled() const;

	/** Callbacks used to determine visibility of the tool usage messages */
	EVisibility GetSelectMessageVisibility() const;
	EVisibility GetWaitMessageVisibility() const;
	EVisibility GetResultVisibility() const;
	
	/** Create a new text collection asset */
	UWitTextCollectionAsset* CreateTextCollectionAsset();

	/** The name entered by the user when creating a new text collection */
	FText NewCollectionText{};

	/** Details widget that will display the settings object */
	TSharedPtr<IDetailsView> DetailsContentWidget{};

	/** Details widget that will display the text collection selection */
	TSharedPtr<IDetailsView> DetailsWidget{};

	/** The currently selected text collection that is being edited */
	UWitEditedTextCollection* EditedTextCollection{};
	
};
