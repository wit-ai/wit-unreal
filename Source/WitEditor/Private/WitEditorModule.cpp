/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitEditorModule.h"

#include "LevelEditor.h"
#include "Wit/TTS/WitTtsService.h"
#include "Customization/TtsConfigurationDetailCustomization.h"
#include "PropertyEditorModule.h"
#include "Tool/ConfigurationEditor/WitConfigurationDetailCustomization.h"
#include "Tool/ConfigurationEditor/WitConfigurationEditorTabTool.h"
#include "Tool/SpeechGenerator/WitSpeechGeneratorTabTool.h"
#include "Tool/UnderstandingViewer/WitUnderstandingViewerTabTool.h"
#include "Tool/UnderstandingViewer/WitUnderstandingPropertyCustomization.h"
#include "Wit/TTS/WitTtsExperience.h"

#define LOCTEXT_NAMESPACE "FWitEditorModule"

IMPLEMENT_MODULE(FWitEditorModule, WitEditor)

TSharedRef<FWorkspaceItem> FWitEditorModule::MenuRoot = FWorkspaceItem::NewGroup(FText::FromString("Menu Root"));

/**
 * Perform module initialization
 */
void FWitEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("Wit Editor Module loaded"));

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyEditorModule.RegisterCustomPropertyTypeLayout( FWitIntent::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FWitUnderstandingPropertyCustomization::MakeInstance) );
	PropertyEditorModule.RegisterCustomClassLayout( ATtsExperience::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FTtsConfigurationDetailCustomization::MakeInstance) );
	PropertyEditorModule.RegisterCustomClassLayout( UWitAppConfigurationAsset::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FWitConfigurationDetailCustomization::MakeInstance) );

	// After all customizations inform the property module to update
	
	PropertyEditorModule.NotifyCustomizationModuleChanged();

	// Add the Oculus menu
	
	if (!IsRunningCommandlet())
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorMenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
		
		MenuExtender = MakeShareable(new FExtender);
		MenuExtender->AddMenuBarExtension("Help", EExtensionHook::Before, nullptr, FMenuBarExtensionDelegate::CreateRaw(this, &FWitEditorModule::AddOculusMenu));

		LevelEditorMenuExtensibilityManager->AddExtender(MenuExtender);
	}
	
	IWitEditorModuleInterface::StartupModule();
}

/**
 * Perform module cleanup
 */
void FWitEditorModule::ShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("Wit Editor Module shut down"));

	// Make sure we clean up any customizations we've added
	
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout(FWitIntent::StaticStruct()->GetFName());
		PropertyEditorModule.UnregisterCustomClassLayout(AWitTtsExperience::StaticClass()->GetFName());
		PropertyEditorModule.UnregisterCustomClassLayout(UWitAppConfigurationAsset::StaticClass()->GetFName());

		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	IWitEditorModuleInterface::ShutdownModule();
}

/**
 * Add any custom tools here
 */
void FWitEditorModule::AddModuleListeners()
{
	ModuleListeners.Add(MakeShareable(new FWitConfigurationEditorTabTool));
	ModuleListeners.Add(MakeShareable(new FWitUnderstandingViewerTabTool));
	ModuleListeners.Add(MakeShareable(new FWitSpeechGeneratorTabTool));
}

/**
 * Add an extension to the general top menu
 */
void FWitEditorModule::AddMenuExtension(const FMenuExtensionDelegate& ExtensionDelegate, FName ExtensionHook, const TSharedPtr<FUICommandList>& CommandList, EExtensionHook::Position Position) const
{
	MenuExtender->AddMenuExtension(ExtensionHook, Position, CommandList, ExtensionDelegate);
}

/**
 * Add an Oculus menu
 */
void FWitEditorModule::AddOculusMenu(FMenuBarBuilder& MenuBuilder) const
{
	const FText DisplayName = FText::FromString("Oculus");
	const FText Tooltip = FText::FromString("Oculus tools");
	const FName ExtensionHook = "Oculus";
	const FName TutorialHighlightName = FName(TEXT("OculusMenu"));
	
	MenuBuilder.AddPullDownMenu( DisplayName, Tooltip, FNewMenuDelegate::CreateRaw(this, &FWitEditorModule::FillOculusMenu), ExtensionHook, TutorialHighlightName);
}

/**
 * Fill the Oculus menu
 */
void FWitEditorModule::FillOculusMenu(FMenuBuilder& MenuBuilder) const
{
	const FName VoiceSdkSectionExtensionHook = "OculusVoiceSDK";
	const FText VoiceSdkSectionDisplayName = FText::FromString("Voice SDK");
	
	MenuBuilder.BeginSection(VoiceSdkSectionExtensionHook, VoiceSdkSectionDisplayName);	
	MenuBuilder.EndSection();
}

#undef LOCTEXT_NAMESPACE
