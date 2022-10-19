/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitConfigurationEditorTabTool.h"
#include "WitEditorModule.h"
#include "SWitConfigurationEditorTab.h"
#include "Tool/Tab/WitTabTool.h"

#define LOCTEXT_NAMESPACE "WitSettingsTabTool"

/**
 * Called on module startup
 */
void FWitConfigurationEditorTabTool::OnStartupModule()
{
	FWitTabTool::OnStartupModule();
	
	FWitEditorModule::Get().AddMenuExtension(FMenuExtensionDelegate::CreateRaw(this, &FWitConfigurationEditorTabTool::MakeMenuEntry), FName("OculusVoiceSDK"), nullptr, EExtensionHook::First);
}

/**
 * Called on module shutdown
 */
void FWitConfigurationEditorTabTool::OnShutdownModule()
{
	FWitTabTool::OnShutdownModule();
}

/**
 * Initialize common properties
 */
void FWitConfigurationEditorTabTool::Initialize()
{
	TabName = "ConfigurationEditor";
	TabDisplayName = FText::FromString("Configuration Editor");
	ToolTipText = FText::FromString("Tool to help configure your Wit.ai app");
}

/**
 * Spawn the associated slate tab widget
 */
TSharedRef<SDockTab> FWitConfigurationEditorTabTool::SpawnTab(const FSpawnTabArgs& TabSpawnArgs)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SWitConfigurationEditorTab)
		];

	return SpawnedTab;
}

#undef LOCTEXT_NAMESPACE
