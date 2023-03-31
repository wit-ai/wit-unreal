/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

using System.IO;
using UnrealBuildTool;

public class Wit : ModuleRules
{
	protected bool bPlatformSupportsLibCurl
	{
		get
		{
			return Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) ||
			       Target.IsInPlatformGroup(UnrealPlatformGroup.Unix) ||
			       Target.IsInPlatformGroup(UnrealPlatformGroup.Android);
		}
	}
	
	public Wit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateDefinitions.Add("WITH_CURL_LIBCURL=" + (bPlatformSupportsLibCurl ? "1" : "0"));
		PrivateDefinitions.Add("WITH_CURL_XCURL=0");
		PrivateDefinitions.Add("WITH_CURL=" + (bPlatformSupportsLibCurl ? "1" : "0"));
		PrivateDefinitions.Add("WITH_SSL=1");
		
		PublicIncludePaths.AddRange(
			new string[]
			{
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(EngineDirectory, "Source/Runtime/Online/HTTP/Private")
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Json",
				"JsonUtilities",				
				"Voice",
				"HTTP",
#if UE_5_1_OR_LATER
				"nghttp2",
				"zlib",
#endif				
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"AudioCaptureCore",
				"AudioMixer",
				"AndroidPermission",
				"SSL",
				"OpenSSL",
				"libcurl",
				"Projects",
				"Sockets",
				"SignalProcessing"
			}
			);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
