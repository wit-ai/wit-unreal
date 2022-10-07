/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitUnderstandingViewerTabTool.h"
#include "WitEditorModule.h"
#include "SWitUnderstandingViewerTab.h"
#include "Tool/Tab/WitTabTool.h"

#define LOCTEXT_NAMESPACE "WitUnderstandingViewerTabTool"

/**
 * Called on module startup
 */
void WitUnderstandingViewerTabTool::OnStartupModule()
{
	WitTabTool::OnStartupModule();
	
	FWitEditorModule::Get().AddMenuExtension(FMenuExtensionDelegate::CreateRaw(this, &WitUnderstandingViewerTabTool::MakeMenuEntry), FName("OculusVoiceSDK"), nullptr, EExtensionHook::First);
}

/**
 * Called on module shutdown
 */
void WitUnderstandingViewerTabTool::OnShutdownModule()
{
	WitTabTool::OnShutdownModule();
}

/**
 * Initialize common properties
 */
void WitUnderstandingViewerTabTool::Initialize()
{
	TabName = "UnderstandingViewer";
	TabDisplayName = FText::FromString("Understanding Viewer");
	ToolTipText = FText::FromString("Tool to help discover how your Wit.ai app will respond to inputs");
}

/**
 * Spawn the associated slate tab widget
 */
TSharedRef<SDockTab> WitUnderstandingViewerTabTool::SpawnTab(const FSpawnTabArgs& TabSpawnArgs)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SWitUnderstandingViewerTab)
		];

	return SpawnedTab;
}

#undef LOCTEXT_NAMESPACE
