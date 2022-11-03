/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "SWitConfigurationEditorTab.h"
#include "Misc/EngineVersionComparison.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "TTS/Configuration/TtsVoicePresetAsset.h"
#include "Wit/Utilities/WitConfigurationUtilities.h"

/**
 * Construct the panel for the settings
 *
 * @param InArgs [in] the arguments associated with this tool
 */
void SWitConfigurationEditorTab::Construct(const FArguments& InArgs)
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
	
	EditedConfiguration = NewObject<UWitEditedConfiguration>();
	EditedConfiguration->AddToRoot();
	
	EditedConfiguration->DetailsContentWidget = DetailsContentWidget;

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
	DetailsWidget->SetObject(EditedConfiguration, true);
	
	ChildSlot
	[
		SNew(SScrollBox)
		 
		+ SScrollBox::Slot().VAlign(VAlign_Top).Padding(10)
		[
			SNew(SVerticalBox)

			// Create a new configuration

			+ SVerticalBox::Slot().AutoHeight().Padding(5)
			[
				SNew(STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				.ColorAndOpacity( FLinearColor( 0.5f, 0.5f, 0.5f, 1.0f ) )
				.Text(FText::FromString(TEXT("Create new configuration")))
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
							.Text(FText::FromString(TEXT("Asset Name")))
						]

						+ SHorizontalBox::Slot().FillWidth(0.9f).Padding(0, 1, 10, 1)
						[
							SNew(SEditableTextBox)
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.OnTextChanged(this, &SWitConfigurationEditorTab::OnNewConfigurationTextChanged)
						]
					]
					
					+ SVerticalBox::Slot().Padding(0, 0).AutoHeight()
					[
						SNew(SHorizontalBox)
						
						+ SHorizontalBox::Slot().VAlign(VAlign_Center).FillWidth(0.1f).Padding(10, 0)
						[
							SNew(STextBlock)
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.Text(FText::FromString(TEXT("Server Access Token")))
						]

						+ SHorizontalBox::Slot().FillWidth(0.9f).Padding(0, 1, 10, 1)
						[
							SNew(SEditableTextBox)
							.IsPassword(true)
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.OnTextChanged(this, &SWitConfigurationEditorTab::OnServerTokenTextChanged)
						]
					]

					+ SVerticalBox::Slot().Padding(0, 0).AutoHeight()
					[
						SNew(SHorizontalBox)
			
						+ SHorizontalBox::Slot().HAlign(HAlign_Right).Padding(10,5,10,2)
						[
							SNew(SButton)
							.Text(FText::FromString(TEXT("Create")))
							.IsEnabled(this, &SWitConfigurationEditorTab::IsNewButtonEnabled)
							.OnClicked(this, &SWitConfigurationEditorTab::OnNewButtonClicked)
						]
					]
				]
			]

			+ SVerticalBox::Slot().Padding(0,10)
			[
				SNew(SBox)
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(5)
			[
				SNew(STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				.ColorAndOpacity( FLinearColor( 0.5f, 0.5f, 0.5f, 1.0f ) )
				.Text(FText::FromString(TEXT("Edit existing configuration")))
			]
			
			// Edit an existing configuration

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
							.Text(FText::FromString(TEXT("Create Voice Presets")))
							.IsEnabled(this, &SWitConfigurationEditorTab::IsCreatePresetButtonEnabled)
							.OnClicked(this, &SWitConfigurationEditorTab::OnCreatePresetButtonClicked)
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
	];
}

/**
 * Callback when the create presents button is clicked
 * 
 * @return whether the reply was handled or not
 */
FReply SWitConfigurationEditorTab::OnCreatePresetButtonClicked()
{
	// Create a preset asset from all the combinations of voice name + style
		
	for (const FWitVoiceDefinition& AvailableVoice: EditedConfiguration->Configuration->Application.Data.Voices)
	{
		const FString PresetAssetName = AvailableVoice.Name;

		FString PackagePath = FString::Printf(TEXT("/Wit/Presets/%s"), *PresetAssetName);

#if WITH_VOICESDK
		PackagePath = FString::Printf(TEXT("/VoiceSDK/Presets/%s"), *PresetAssetName);
#endif
			
		UPackage* PresetPackage = CreatePackage(*PackagePath);

		if (PresetPackage == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("OnCreatePresetButtonClicked: failed to create package file for (%s)"), *PresetAssetName);
			continue;
		}

		UTtsVoicePresetAsset* PresetAsset = NewObject<UTtsVoicePresetAsset>(PresetPackage, UTtsVoicePresetAsset::StaticClass(), *PresetAssetName, RF_Public | RF_Standalone);

		if (PresetAsset == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("OnCreatePresetButtonClicked: failed to create asset file for (%s)"), *PresetAssetName);
			continue;
		}

		PresetAsset->Synthesize.Voice = AvailableVoice.Name;
    
		(void)PresetAsset->MarkPackageDirty();
	}

	return FReply::Handled();
}

/**
 * Determines if the create preset button should be enabled or not
 * 
 * @return true if enabled otherwise false
 */
bool SWitConfigurationEditorTab::IsCreatePresetButtonEnabled() const
{
	if (FWitConfigurationUtilities::IsRefreshInProgress())
	{
		return false;
	}

	const bool bIsValidVoiceData = EditedConfiguration != nullptr && EditedConfiguration->Configuration != nullptr && EditedConfiguration->Configuration->Application.Data.Voices.Num() > 0;

	return bIsValidVoiceData;
}

/**
 * Callback when the new button is clicked
 * 
 * @return whether the reply was handled or not
 */
FReply SWitConfigurationEditorTab::OnNewButtonClicked()
{
	UWitAppConfigurationAsset* NewAsset = CreateConfigurationAsset();

	if (EditedConfiguration != nullptr && NewAsset != nullptr)
	{
		EditedConfiguration->Configuration = NewAsset;

		if (DetailsContentWidget != nullptr)
		{
			DetailsContentWidget->SetObject(NewAsset);
		}
	
		FWitConfigurationUtilities::RefreshConfiguration(NewAsset);
	}
	
	return FReply::Handled();
}

/**
 * Determines if the new button should be enabled or not
 * 
 * @return true if enabled otherwise false
 */
bool SWitConfigurationEditorTab::IsNewButtonEnabled() const
{
	return !ServerTokenText.IsEmpty() && !NewConfigurationText.IsEmpty();
}

/**
 * Callback when the server token text is changed
 *
 * @param InText [in] the text that was entered
 */
void SWitConfigurationEditorTab::OnServerTokenTextChanged(const FText& InText)
{
	ServerTokenText = InText;
}

/**
 * Callback when the new configuration text is changed
 *
 * @param InText [in] the text that was entered
 */
void SWitConfigurationEditorTab::OnNewConfigurationTextChanged(const FText& InText)
{
	NewConfigurationText = InText;
}

/*
 * Create a new configuration asset
 */
UWitAppConfigurationAsset* SWitConfigurationEditorTab::CreateConfigurationAsset()
{
	const FString PackagePath = FString::Printf(TEXT("/Game/%s"), *NewConfigurationText.ToString());
	
	UPackage* Package = CreatePackage(*PackagePath);

	if (Package == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SWitConfigurationEditorTab::CreateConfigurationAsset: failed to create package file for (%s)"), *NewConfigurationText.ToString());
		return nullptr;
	}
	
	UWitAppConfigurationAsset* Asset = NewObject<UWitAppConfigurationAsset>(Package, UWitAppConfigurationAsset::StaticClass(), *NewConfigurationText.ToString(), RF_Public | RF_Standalone);

	if (Asset == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SWitConfigurationEditorTab::CreateConfigurationAsset: failed to create asset file for (%s)"), *NewConfigurationText.ToString());
		return nullptr;
	}

	Asset->Application.ServerAccessToken = ServerTokenText.ToString();
	
	(void)Asset->MarkPackageDirty();

	return Asset;
}
