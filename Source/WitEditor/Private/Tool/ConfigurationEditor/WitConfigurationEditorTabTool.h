/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "Tool/Tab/WitTabTool.h"

class FWitConfigurationEditorTabTool : public FWitTabTool
{
public:
	
	virtual ~FWitConfigurationEditorTabTool() override {}

	/** IWitEditorModuleListenerInterface implementation */
	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;

protected:
	
	/** WitTabTool implementation */
	virtual void Initialize() override;
	virtual TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& TabSpawnArgs) override;
	
};
