/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "IWitEditorModuleInterface.h"
#include "WitEditorModule.h"

class FWitTabTool : public IWitEditorModuleListenerInterface, public TSharedFromThis<FWitTabTool>
{
public:
	
	/** IWitEditorModuleListenerInterface implementation */
	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;

protected:

	/** Override these to initialize and spawn the associated tab */
	virtual void Initialize() = 0;
	virtual TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& TabSpawnArgs) = 0;

	/** Populate the menu entry */
	virtual void MakeMenuEntry(FMenuBuilder& MenuBuilder);
	
	/** Internal name to use for the tab */
	FName TabName;

	/** Display name to use for the tab */
	FText TabDisplayName;

	/** Tool tip text for the tab */
	FText ToolTipText;
	
};
