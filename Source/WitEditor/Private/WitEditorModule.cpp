/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitEditorModule.h"
#include "Wit/Voice/WitVoiceService.h"
#include "Wit/TTS/WitTtsService.h"
#include "Customization/WitResponseDetailCustomization.h"
#include "Customization/WitIntentPropertyCustomization.h"
#include "Customization/TtsConfigurationDetailCustomization.h"
#include "PropertyEditor/Public/PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FWitEditorModule"

IMPLEMENT_MODULE(FWitEditorModule, WitEditor)

/**
 * Perform module initialization
 */
void FWitEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("Wit Editor Module loaded"));

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Register a custom class layout for the WitVoiceCommandAPI. This is used to add the understanding viewer to the response
	// so that we can easily debug and use Wit.ai
	
	PropertyEditorModule.RegisterCustomClassLayout( UVoiceEvents::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FWitResponseDetailCustomization::MakeInstance) );

	// Register a custom property layout for the WitIntent and WitEntity. This is used to add buttons so we can add
	// handlers to specific response entities and intents

	PropertyEditorModule.RegisterCustomPropertyTypeLayout( FWitIntent::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FWitIntentPropertyCustomization::MakeInstance) );
	
	// Register a custom class layout for the WitTextToSpeechAPI
	// so that we can easily debug and use Wit.ai
	
	PropertyEditorModule.RegisterCustomClassLayout( UWitTtsService::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FTtsConfigurationDetailCustomization::MakeInstance) );

	// After all customizations inform the property module to update
	
	PropertyEditorModule.NotifyCustomizationModuleChanged();
}

/**
 * Perform module cleanup
 */
void FWitEditorModule::ShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("WIT Editor Module shut down"));

	// Make sure we clean up any customizations we've added
	
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		
		PropertyEditorModule.UnregisterCustomClassLayout(UVoiceEvents::StaticClass()->GetFName());
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout(FWitIntent::StaticStruct()->GetFName());
		PropertyEditorModule.UnregisterCustomClassLayout(UWitTtsService::StaticClass()->GetFName());

		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}
}

#undef LOCTEXT_NAMESPACE
