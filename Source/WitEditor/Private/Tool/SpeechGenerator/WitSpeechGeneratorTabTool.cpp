/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitSpeechGeneratorTabTool.h"
#include "WitEditorModule.h"
#include "SWitSpeechGeneratorTab.h"
#include "Tool/Tab/WitTabTool.h"

#define LOCTEXT_NAMESPACE "WitSpeechGeneratorTabTool"

/**
 * Called on module startup
 */
void FWitSpeechGeneratorTabTool::OnStartupModule()
{
	FWitTabTool::OnStartupModule();
	
	FWitEditorModule::Get().AddMenuExtension(FMenuExtensionDelegate::CreateRaw(this, &FWitSpeechGeneratorTabTool::MakeMenuEntry), FName("OculusVoiceSDK"), nullptr, EExtensionHook::First);
}

/**
 * Called on module shutdown
 */
void FWitSpeechGeneratorTabTool::OnShutdownModule()
{
	FWitTabTool::OnShutdownModule();
}

/**
 * Initialize common properties
 */
void FWitSpeechGeneratorTabTool::Initialize()
{
	TabName = "SpeechGenerator";
	TabDisplayName = FText::FromString("Speech Generator");
	ToolTipText = FText::FromString("Tool to help bulk generation of speech from text");
}

/**
 * Spawn the associated slate tab widget
 */
TSharedRef<SDockTab> FWitSpeechGeneratorTabTool::SpawnTab(const FSpawnTabArgs& TabSpawnArgs)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SWitSpeechGeneratorTab)
		];

	return SpawnedTab;
}

#undef LOCTEXT_NAMESPACE
