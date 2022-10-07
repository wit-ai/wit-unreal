/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

class IWitEditorModuleListenerInterface
{
public:

	virtual ~IWitEditorModuleListenerInterface() {};
	
	virtual void OnStartupModule() {};
	virtual void OnShutdownModule() {};

};

class IWitEditorModuleInterface : public IModuleInterface
{
public:
	
	virtual void StartupModule() override
	{
		if (!IsRunningCommandlet())
		{
			AddModuleListeners();
			
			for (int32 i = 0; i < ModuleListeners.Num(); ++i)
			{
				ModuleListeners[i]->OnStartupModule();
			}
		}
	}

	virtual void ShutdownModule() override
	{
		for (int32 i = 0; i < ModuleListeners.Num(); ++i)
		{
			ModuleListeners[i]->OnShutdownModule();
		}
	}

	virtual void AddModuleListeners() {};

protected:
	
	TArray<TSharedRef<IWitEditorModuleListenerInterface>> ModuleListeners;
	
};
