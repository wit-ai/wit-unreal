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
#include "EditorStyleSet.h"
#include "Tool/Utilities/WitEditorHelperUtilities.h"
#include "TTS/Cache/Storage/TtsStorageCache.h"
#include "Wit/Utilities/WitHelperUtilities.h"

#define LOCTEXT_NAMESPACE "SWitSpeechGeneratorTab"

/**
 * Construct the panel for the speech generator
 *
 * @param InArgs [in] the arguments associated with this tool
 */
void SWitSpeechGeneratorTab::Construct(const FArguments& InArgs)
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs ContentArgs;
	
	ContentArgs.bAllowSearch = false;
	ContentArgs.bAllowFavoriteSystem = false;
	ContentArgs.bHideSelectionTip = true;
#if UE_VERSION_OLDER_THAN(5,0,0)
	ContentArgs.bShowActorLabel = true;
#else
	ContentArgs.bShowObjectLabel = true;
#endif

	ContentArgs.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	ContentArgs.ColumnWidth = 0.5f;

	DetailsContentWidget = PropertyModule.CreateDetailView(ContentArgs);
	DetailsContentWidget->SetVisibility(EVisibility::Visible);
	
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
	DetailsWidget->SetVisibility(EVisibility::Visible);

	EditedTextCollection = NewObject<UWitEditedTextCollection>();
	EditedTextCollection->AddToRoot();
	
	DetailsWidget->SetObject(EditedTextCollection, true);
	EditedTextCollection->DetailsContentWidget = DetailsContentWidget;
	
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
				.Text(LOCTEXT("CreateNewTextCollection", "Create new text collection"))
			]
			
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBorder)
				.Padding(5)
				.BorderImage( WitEditorHelperUtilities::GetBrush("ToolPanel.GroupBorder") )
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
							.ToolTipText(LOCTEXT("AssetNameTooltip", "File name for the new text collection asset. This can contain a path relative to the root content folder."))
							.Text(LOCTEXT("AssetName", "Collection Name"))
						]

						+ SHorizontalBox::Slot().FillWidth(0.9f).Padding(0, 1, 10, 1)
						[
							SNew(SEditableTextBox)
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.OnTextCommitted(this, &SWitSpeechGeneratorTab::OnNewCollectionTextCommitted)
							.OnTextChanged(this, &SWitSpeechGeneratorTab::OnNewCollectionTextChanged)
						]
					]

					+ SVerticalBox::Slot().Padding(0, 0).AutoHeight()
					[
						SNew(SHorizontalBox)
			
						+ SHorizontalBox::Slot().HAlign(HAlign_Right).Padding(10,5,10,2)
						[
							SNew(SButton)
							.ToolTipText(LOCTEXT("CreateTooltip", "Create a new empty text collection."))
							.Text(LOCTEXT("CreateButton", "Create"))
							.IsEnabled(this, &SWitSpeechGeneratorTab::IsNewButtonEnabled)
							.OnClicked(this, &SWitSpeechGeneratorTab::OnNewButtonClicked)
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
					.Text(LOCTEXT("SpeechGeneratorUsage1", "Select a TTS Experience actor to begin."))
					.Visibility(this, &SWitSpeechGeneratorTab::GetSelectMessageVisibility)
				]
			
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Fill).Padding(12.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SpeechGeneratorUsage2", "Waiting for conversion to finish..."))
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
						.Text(LOCTEXT("EditTextCollection", "Edit existing text collection"))
					]

					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBorder)
						.Padding(5)
						.BorderImage( WitEditorHelperUtilities::GetBrush("ToolPanel.GroupBorder") )
						.Content()
						[
							SNew(SVerticalBox)
					
							+ SVerticalBox::Slot().Padding(0, 0).AutoHeight()
							[
								SNew(SOverlay)

								+ SOverlay::Slot()
								[
									DetailsWidget.ToSharedRef()
								]
							]

							+ SVerticalBox::Slot().Padding(0, 0).AutoHeight()
							[
								SNew(SHorizontalBox)
								
								+ SHorizontalBox::Slot().HAlign(HAlign_Right).Padding(10,5,10,2)
								[
									SNew(SButton)
									.ToolTipText(LOCTEXT("GenerateAllButtonTooltip", "Generates all the sound assets from text."))
									.Text(LOCTEXT("GenerateAllButton", "Generate All"))
									.IsEnabled(this, &SWitSpeechGeneratorTab::IsConvertButtonEnabled)
									.OnClicked(this, &SWitSpeechGeneratorTab::OnConvertButtonClicked)
								]
							]
							
							+ SVerticalBox::Slot().Padding(0, 0)
							[
								SNew(SOverlay)

								+ SOverlay::Slot()
								[
									DetailsContentWidget.ToSharedRef()
								]
							]
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

	if (TtsExperience->IsRequestInProgress() || EditedTextCollection->bIsConvertInProgress)
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
	if (EditedTextCollection->TextCollection == nullptr)
	{
		return FReply::Handled();
	}
	
	if (EditedTextCollection->TextCollection->Items.Num() == 0)
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
	TtsService->SetConfiguration(TtsExperience->Configuration, nullptr, EWitRequestAudioFormat::Wav, false, 1.0f, false);
	
	TtsExperience->EventHandler->OnSynthesizeRawResponse.AddUniqueDynamic(EditedTextCollection, &UWitEditedTextCollection::OnSynthesizeRawResponse);
	TtsExperience->EventHandler->OnSynthesizeError.AddUniqueDynamic(EditedTextCollection, &UWitEditedTextCollection::OnSynthesizeError);
	
	EditedTextCollection->CurrentConvertIndex = 0;
	EditedTextCollection->CurrentConvertIndex = EditedTextCollection->FindNextItemToConvert();

	const bool bIsMoreItemsToConvert = EditedTextCollection->CurrentConvertIndex < EditedTextCollection->TextCollection->Items.Num();
	
	if (bIsMoreItemsToConvert)
	{
		// We must invalidate the memory cache to ensure that it is bypassed and the clips always get regenerated

		TtsExperience->UnloadAllClips();
		
		EditedTextCollection->bIsConvertInProgress = true;
		EditedTextCollection->ConvertTextItem(EditedTextCollection->TextCollection->Items[EditedTextCollection->CurrentConvertIndex]);
	}
	
	return FReply::Handled();
}

/**
 * Determines if the convert button should be enabled or not
 * 
 * @return true if enabled otherwise false
 */
bool SWitSpeechGeneratorTab::IsConvertButtonEnabled() const
{
	const bool bIsItemsToConvert = EditedTextCollection->TextCollection != nullptr && EditedTextCollection->TextCollection->Items.Num() > 0;

	if (!bIsItemsToConvert)
	{
		return false;
	}

	const ATtsExperience* TtsExperience = GetSelectedTtsExperience();

	return TtsExperience != nullptr && !EditedTextCollection->bIsConvertInProgress;
}

/**
 * Callback when the new button is clicked
 * 
 * @return whether the reply was handled or not
 */
FReply SWitSpeechGeneratorTab::OnNewButtonClicked()
{
	UWitTextCollectionAsset* NewAsset = CreateTextCollectionAsset();

	if (EditedTextCollection != nullptr && NewAsset != nullptr)
	{
		EditedTextCollection->TextCollection = NewAsset;

		if (DetailsContentWidget != nullptr)
		{
			DetailsContentWidget->SetObject(NewAsset);
		}
	}
	
	return FReply::Handled();
}

/**
 * Determines if the new button should be enabled or not
 * 
 * @return true if enabled otherwise false
 */
bool SWitSpeechGeneratorTab::IsNewButtonEnabled() const
{
	return !NewCollectionText.IsEmpty();
}

/**
 * Callback when the new collection text box loses focus ("return" not able to trigger this )
 * 
 * @param InText [in] the text that was entered 
 */
void SWitSpeechGeneratorTab::OnNewCollectionTextCommitted(const FText& InText, ETextCommit::Type)
{
	NewCollectionText = InText;
}

/**
 * Callback when the new collection text is changed
 *
 * @param InText [in] the text that was entered
 */
void SWitSpeechGeneratorTab::OnNewCollectionTextChanged(const FText& InText)
{
	NewCollectionText = InText;
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

	return Cast<ATtsExperience>(SelectedActors->GetSelectedObject(0));
}

/**
 * Callback that is called when a Wit.ai synthesize response is received
 */
void UWitEditedTextCollection::OnSynthesizeRawResponse(const FString& ClipId, const TArray<uint8>& BinaryData, const FTtsConfiguration& ClipSettings)
{
	FWitHelperUtilities::SaveClipToAssetFile(TextCollection->ContentFolder, ClipId, BinaryData, ClipSettings);

	++CurrentConvertIndex;
	CurrentConvertIndex = FindNextItemToConvert();
	
	const bool bIsMoreItemsToConvert = CurrentConvertIndex < TextCollection->Items.Num();

	if (bIsMoreItemsToConvert)
	{
		ConvertTextItem(TextCollection->Items[CurrentConvertIndex]);
	}
	else
	{
		bIsConvertInProgress = false;
	}
}

/**
 * Callback when an error occurs
 */
void UWitEditedTextCollection::OnSynthesizeError(const FString& ErrorMessage, const FString& HumanReadableMessage)
{
	++CurrentConvertIndex;
	CurrentConvertIndex = FindNextItemToConvert();
	
	const bool bIsMoreItemsToConvert = CurrentConvertIndex < TextCollection->Items.Num();

	if (bIsMoreItemsToConvert)
	{
		ConvertTextItem(TextCollection->Items[CurrentConvertIndex]);
	}
	else
	{
		bIsConvertInProgress = false;
	}
}

/**
 * Convert a single text item
 */
void UWitEditedTextCollection::ConvertTextItem(FWitTextItem& ItemToConvert)
{
	FTtsConfiguration ClipSettings = ItemToConvert.VoicePreset->Synthesize;

	ClipSettings.Text = ItemToConvert.Text;
	ItemToConvert.ClipId = FWitHelperUtilities::GetVoiceClipId(ClipSettings);

	ATtsExperience* TtsExperience = SWitSpeechGeneratorTab::GetSelectedTtsExperience();
	
	TtsExperience->ConvertTextToSpeechWithSettings(ClipSettings);
}

/**
 * Find the next valid item to convert
 */
int32 UWitEditedTextCollection::FindNextItemToConvert()
{
	while (CurrentConvertIndex < TextCollection->Items.Num())
	{
		const FWitTextItem& ItemToConvert = TextCollection->Items[CurrentConvertIndex];
		const bool bIsValidItem = ItemToConvert.VoicePreset != nullptr && ItemToConvert.Text.Len() > 0;

		if (bIsValidItem)
		{
			break;
		}

		++CurrentConvertIndex;
	}

	return CurrentConvertIndex;
}

/*
 * Create a new text collection asset
 */
UWitTextCollectionAsset* SWitSpeechGeneratorTab::CreateTextCollectionAsset()
{
	const FString AssetNameText = NewCollectionText.ToString();
	const FString PackagePath = FString::Printf(TEXT("/Game/%s"), *AssetNameText);
	
	UPackage* Package = CreatePackage(*PackagePath);
	const FString PackageName = FPaths::GetBaseFilename(PackagePath);

	if (Package == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SWitConfigurationEditorTab::CreateConfigurationAsset: failed to create package file for (%s)"), *AssetNameText);
		return nullptr;
	}
	
	UWitTextCollectionAsset* Asset = NewObject<UWitTextCollectionAsset>(Package, UWitTextCollectionAsset::StaticClass(), *PackageName, RF_Public | RF_Standalone);

	if (Asset == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SWitConfigurationEditorTab::CreateConfigurationAsset: failed to create asset file for (%s)"), *AssetNameText);
		return nullptr;
	}
	
	(void)Asset->MarkPackageDirty();

	return Asset;
}

#undef LOCTEXT_NAMESPACE
