/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitTabTool.h"

/**
 * Called on module startup
 */
void WitTabTool::OnStartupModule()
{
	Initialize();
		
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName, FOnSpawnTab::CreateRaw(this, &WitTabTool::SpawnTab))
		.SetGroup(FWitEditorModule::Get().GetMenuRoot())
		.SetDisplayName(TabDisplayName)
		.SetTooltipText(ToolTipText);
};

/**
 * Called on module shutdown
 */
void WitTabTool::OnShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
};

/**
 * Populate the menu entry 
 */
void WitTabTool::MakeMenuEntry(FMenuBuilder& MenuBuilder)
{
	FGlobalTabmanager::Get()->PopulateTabSpawnerMenu(MenuBuilder, TabName);
};
