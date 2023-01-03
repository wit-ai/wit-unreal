/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

using UnrealBuildTool;

public class WitEditor : ModuleRules
{
	public WitEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDefinitions.Add("WITH_VOICESDK=0");
		PrivateDefinitions.Add("WITH_VOICESDK_MARKETPLACE=0");
		
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"HTTP",
				"InputCore",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"EditorWidgets",
				"PropertyEditor",
				"EditorStyle",
				"Wit",
			});
	}
}
