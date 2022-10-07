/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "IWitEditorModuleInterface.h"

/**
 * The module for the UE4 editor plugin implementation of the Wit API
 */
class FWitEditorModule final : public IWitEditorModuleInterface
{
public:
	
	/**
	 * IWitEditorModuleInterface implementation
	 */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual void AddModuleListeners() override;

	/** Get the module */
	static FWitEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FWitEditorModule>("WitEditor");
	}

	/** Add the main menu extension */	
	void AddMenuExtension(const FMenuExtensionDelegate& ExtensionDelegate, FName ExtensionHook, const TSharedPtr<FUICommandList>& CommandList = nullptr, EExtensionHook::Position Position = EExtensionHook::Before) const;

	/** Access the menu root */
	static TSharedRef<FWorkspaceItem> GetMenuRoot() { return MenuRoot; }

protected:
	
	TSharedPtr<FExtensibilityManager> LevelEditorMenuExtensibilityManager;
	TSharedPtr<FExtender> MenuExtender;

	static TSharedRef<FWorkspaceItem> MenuRoot;

	void AddOculusMenu(FMenuBarBuilder& MenuBuilder) const;
	void FillOculusMenu(FMenuBuilder& MenuBuilder) const;
	
};
