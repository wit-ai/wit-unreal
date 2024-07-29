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
		bEnableExceptions = true;

		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDefinitions.Add("WITH_CURL_LIBCURL=" + (bPlatformSupportsLibCurl ? "1" : "0"));
		PrivateDefinitions.Add("WITH_CURL_XCURL=0");
		PrivateDefinitions.Add("WITH_CURL_MULTIPOLL=0");
		PrivateDefinitions.Add("WITH_CURL_MULTIWAIT=0");
		PrivateDefinitions.Add("WITH_CURL_MULTISOCKET=0");
		PrivateDefinitions.Add("WITH_CURL=" + (bPlatformSupportsLibCurl ? "1" : "0"));
		PrivateDefinitions.Add("WITH_CURL_QUICKEXIT=1");
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
				"WebSockets",
#if UE_5_1_OR_LATER
				"nghttp2",
				"zlib",
#if UE_5_4_OR_LATER
				"EventLoop",
#endif	
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

		string ThirdPartyDir = Path.Combine(ModuleDirectory, "..", "ThirdParty");
		PublicIncludePaths.Add(Path.Combine(ThirdPartyDir, "VoiceSDK", "include"));
		PublicIncludePaths.Add(Path.Combine(ThirdPartyDir, "boost", "include"));
		PublicIncludePaths.Add(Path.Combine(ThirdPartyDir, "double-conversion", "include"));
		PublicIncludePaths.Add(Path.Combine(ThirdPartyDir, "fmt", "include"));
		PublicIncludePaths.Add(Path.Combine(ThirdPartyDir, "folly", "include"));
		PublicIncludePaths.Add(Path.Combine(ThirdPartyDir, "glog", "include"));
		PublicIncludePaths.Add(Path.Combine(ThirdPartyDir, "gtest", "include"));
		PublicIncludePaths.Add(Path.Combine(ThirdPartyDir, "magic_enum", "include"));
		PublicIncludePaths.Add(Path.Combine(ThirdPartyDir, "thrift", "include"));
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string VoiceSdkLibraryPath = Path.Combine(ThirdPartyDir, "VoiceSDK", "lib", "Android");
			PublicAdditionalLibraries.Add(Path.Combine(VoiceSdkLibraryPath, "libvoicesdk.a"));
			PublicAdditionalLibraries.Add(Path.Combine(VoiceSdkLibraryPath, "libstubs.pic.a"));
			PublicAdditionalLibraries.Add(Path.Combine(VoiceSdkLibraryPath, "libutils.pic.a"));

			PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyDir, "double-conversion", "lib", "Android", "lib_double-conversion.pic.a"));

			string FollyLibraryPath = Path.Combine(ThirdPartyDir, "folly", "lib", "Android");
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libasync_base.pic.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libc_string.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libdemangle.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libdynamic.pic.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libexception_string.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libexception.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libexecutor.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libjson.pic.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libunicode.pic.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libhash_hash.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libjson_pointer.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libformat.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libconv.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libio_iobuf.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libto_ascii.a"));
			PublicAdditionalLibraries.Add(Path.Combine(FollyLibraryPath, "libsafe_assert.a"));

			PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyDir, "glog", "lib", "Android", "lib_glog.pic.a"));
		}

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
