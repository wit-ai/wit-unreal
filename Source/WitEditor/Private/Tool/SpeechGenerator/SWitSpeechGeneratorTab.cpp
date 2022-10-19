/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "SWitSpeechGeneratorTab.h"
#include "Misc/EngineVersionComparison.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Engine/Selection.h"
#include "Editor.h"
#include "TTS/Cache/Storage/TtsStorageCache.h"
#include "Wit/Utilities/WitHelperUtilities.h"

/**
 * Construct the panel for the understanding viewer
 *
 * @param InArgs [in] the arguments associated with this tool
 */
void SWitSpeechGeneratorTab::Construct(const FArguments& InArgs)
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	FDetailsViewArgs Args;
	
	Args.bAllowSearch = false;
	Args.bAllowFavoriteSystem = false;
	Args.bHideSelectionTip = true;
#if UE_VERSION_OLDER_THAN(5,0,0)
	Args.bShowActorLabel = false;
#else
	Args.bShowObjectLabel = false;
#endif
	Args.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	Args.ColumnWidth = 0.5f;

	DetailsWidget = PropertyModule.CreateDetailView(Args);
	DetailsWidget->SetVisibility(EVisibility::Hidden);

	TextCollection = NewObject<UWitTextCollection>();
	
	DetailsWidget->SetObject(TextCollection, true);
	
	ChildSlot
	[
		SNew(SScrollBox)
		 
		+ SScrollBox::Slot().VAlign(VAlign_Top).Padding(10)
		[
			SNew(SVerticalBox)

			// Section to contain the text input and send button
			
			+ SVerticalBox::Slot().AutoHeight().Padding(5)
			[
				SNew(STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				.ColorAndOpacity( FLinearColor( 0.5f, 0.5f, 0.5f, 1.0f ) )
				.Text(FText::FromString(TEXT("Enter output location")))
			]
			
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBorder)
				.Padding(5)
				.BorderImage( FEditorStyle::GetBrush("ToolPanel.GroupBorder") )
				.Content()
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot().Padding(0, 0).AutoHeight()
					[
						SNew(SHorizontalBox)
						
						+ SHorizontalBox::Slot().VAlign(VAlign_Center).FillWidth(0.1f).Padding(10, 0)
						[
							SNew(STextBlock)
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.Text(FText::FromString(TEXT("Content folder")))
						]

						+ SHorizontalBox::Slot().FillWidth(0.9f).Padding(0, 1, 10, 1)
						[
							SNew(SEditableTextBox)
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.OnTextCommitted(this, &SWitSpeechGeneratorTab::OnOutputLocationTextCommitted)
							.OnTextChanged(this, &SWitSpeechGeneratorTab::OnOutputLocationTextChanged)
						]
					]

					+ SVerticalBox::Slot().Padding(0, 0).AutoHeight()
					[
						SNew(SHorizontalBox)
			
						+ SHorizontalBox::Slot().HAlign(HAlign_Right).Padding(10,5,10,2)
						[
							SNew(SButton)
							.Text(FText::FromString(TEXT("Convert")))
							.IsEnabled(this, &SWitSpeechGeneratorTab::IsConvertButtonEnabled)
							.OnClicked(this, &SWitSpeechGeneratorTab::OnConvertButtonClicked)
						]
					]
				]
			]	

			+ SVerticalBox::Slot().Padding(0,10)
			[
				SNew(SBox)
			]
			
			// Section to contain both the usage messaging and the actual response from Wit.ai
			
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SOverlay)

				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Fill).Padding(12.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Select a TTS Experience actor to begin.")))
					.Visibility(this, &SWitSpeechGeneratorTab::GetSelectMessageVisibility)
				]
			
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Fill).Padding(12.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Waiting for response...")))
					.Visibility(this, &SWitSpeechGeneratorTab::GetWaitMessageVisibility)
				]

				+ SOverlay::Slot()
				[
					SNew(SVerticalBox)
					.Visibility(this, &SWitSpeechGeneratorTab::GetResultVisibility)
										
					+ SVerticalBox::Slot().AutoHeight().Padding(5)
					[
						SNew(STextBlock)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
						.ColorAndOpacity( FLinearColor( 0.5f, 0.5f, 0.5f, 1.0f ) )
						.Text(FText::FromString(TEXT("Convert text")))
					]

					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBorder)
						.Padding(5)
						.BorderImage( FEditorStyle::GetBrush("ToolPanel.GroupBorder") )
						.Content()
						[
							DetailsWidget.ToSharedRef()
						]
					]
				]	
			]
		]
	];
}

/**
 * Whether we should show the selection usage message
 * 
 * @return true if we should show otherwise false
 */
EVisibility SWitSpeechGeneratorTab::GetSelectMessageVisibility() const
{
	if (GetWaitMessageVisibility() == EVisibility::Visible)
	{
		return EVisibility::Hidden;
	}

	const ATtsExperience* TtsExperience = GetSelectedTtsExperience();
	
	if (TtsExperience == nullptr)
	{
		return EVisibility::Visible;	
	}
	
	return EVisibility::Hidden;
}

/**
 * Whether we should show the waiting usage message
 * 
 * @return true if we should show otherwise false
 */
EVisibility SWitSpeechGeneratorTab::GetWaitMessageVisibility() const
{
	const ATtsExperience* TtsExperience = GetSelectedTtsExperience();
	
	if (TtsExperience == nullptr)
	{
		return EVisibility::Hidden;	
	}

	if (TtsExperience->IsRequestInProgress() || TextCollection->bIsConvertInProgress)
	{
		return EVisibility::Visible;
	}
	
	return EVisibility::Hidden;
}

/**
 * Whether we should show the result
 * 
 * @return true if we should show otherwise false
 */
EVisibility SWitSpeechGeneratorTab::GetResultVisibility() const
{
	if (GetSelectMessageVisibility() == EVisibility::Visible)
	{
		return EVisibility::Hidden;
	}
	
	if (GetWaitMessageVisibility() == EVisibility::Visible)
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

/**
 * Callback when the convert button is clicked
 * 
 * @return whether the reply was handled or not
 */
FReply SWitSpeechGeneratorTab::OnConvertButtonClicked()
{
	if (TextCollection->Items.Num() == 0)
	{
		return FReply::Handled();
	}
	
	ATtsExperience* TtsExperience = GetSelectedTtsExperience();
	
	if (TtsExperience == nullptr || TtsExperience->TtsService == nullptr || TtsExperience->EventHandler == nullptr)
	{
		return FReply::Handled();
	}
		
	if (TtsExperience->IsRequestInProgress())
	{
		return FReply::Handled();
	}
	
	UTtsService* TtsService = TtsExperience->TtsService;

	TtsService->SetHandlers(TtsExperience->EventHandler, nullptr, nullptr);
	TtsService->SetConfiguration(TtsExperience->Configuration, nullptr);
	
	TtsExperience->EventHandler->OnSynthesizeRawResponse.AddUniqueDynamic(TextCollection, &UWitTextCollection::OnSynthesizeRawResponse);

	TextCollection->bIsConvertInProgress = true;
	TextCollection->CurrentConvertIndex = 0;

	TextCollection->ConvertTextItem(TextCollection->Items[0]);
	
	return FReply::Handled();
}

/**
 * Determines if the convert button should be enabled or not
 * 
 * @return true if enabled otherwise false
 */
bool SWitSpeechGeneratorTab::IsConvertButtonEnabled() const
{
	const ATtsExperience* TtsExperience = GetSelectedTtsExperience();

	return TtsExperience != nullptr && !TextCollection->bIsConvertInProgress;
}

/**
 * Callback when the output location text box loses focus ("return" not able to trigger this )
 * 
 * @param InText [in] the text that was entered 
 */
void SWitSpeechGeneratorTab::OnOutputLocationTextCommitted(const FText& InText, ETextCommit::Type)
{
	TextCollection->ContentOutputLocation = InText;
}

/**
 * Callback when the output location text is changed
 *
 * @param InText [in] the text that was entered
 */
void SWitSpeechGeneratorTab::OnOutputLocationTextChanged(const FText& InText)
{
	TextCollection->ContentOutputLocation = InText;
}

/**
 * Gets the selected WitTtsExperience in the scene (if any)
 * 
 * @return pointer to the Tts Experience actor if selected otherwise null
 */
ATtsExperience* SWitSpeechGeneratorTab::GetSelectedTtsExperience()
{
	const bool bIsSingleSelectedActor = GEditor->GetSelectedActorCount() == 1;
	
	if (!bIsSingleSelectedActor)
	{
		return nullptr;
	}

	USelection* SelectedActors = GEditor->GetSelectedActors();

	for(FSelectionIterator It(*SelectedActors); It; ++It)
	{
		return Cast<ATtsExperience>(*It);
	}

	return nullptr;
}

/**
 * Callback that is called when a Wit.ai response is received
 */
void UWitTextCollection::OnSynthesizeRawResponse(const FString& ClipId, const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings)
{
	FWitHelperUtilities::SaveClipToAssetFile(ContentOutputLocation.ToString(), ClipId, BinaryData, ClipSettings);

	++CurrentConvertIndex;
	const bool bIsMoreItemsToConvert = CurrentConvertIndex < Items.Num();

	if (bIsMoreItemsToConvert)
	{		
		ConvertTextItem(Items[CurrentConvertIndex]);
	}
	else
	{
		bIsConvertInProgress = false;
	}
}

/**
 * Convert a single text item
 */
void UWitTextCollection::ConvertTextItem(FWitTextItem& ItemToConvert)
{
	FTtsConfiguration ClipSettings = ItemToConvert.VoicePreset->Synthesize;

	ClipSettings.Text = ItemToConvert.Text;
	ItemToConvert.ClipId = FWitHelperUtilities::GetVoiceClipId(ClipSettings);

	ATtsExperience* TtsExperience = SWitSpeechGeneratorTab::GetSelectedTtsExperience();
	
	TtsExperience->ConvertTextToSpeechWithSettings(ClipSettings);
}
