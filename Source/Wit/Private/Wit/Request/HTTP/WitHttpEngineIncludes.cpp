/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define UI UI_ST

// HTTP engine files - included here because these are private files but we want to inherit from them
// This is not pretty nor good but is the only way to extend these classes without modifying engine code

// The approach we take here is that we want to use the CurlHttp provided by Unreal but we need to make a few
// changes and so can't use it directly. We also do not want to impose engine changes.

// The changes we need to make are in order to support HTTP chunked transfers:
//
// 1. Prevent the Content-Length header being automatically added as this is not needed for chunked transfers
// 2. Reset the post field size to default so that it doesn't prematurely stop the transfer
// 3. Add support for pausing and resuming requests using our own read function

#if WITH_CURL && WITH_EDITOR
#include "Http.h"
#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_OLDER_THAN(5, 4, 0)
#else
#undef HTTP_API
#define HTTP_API DLLEXPORT
#endif

#if UE_VERSION_OLDER_THAN(5, 5, 0)
#else
// These defines are private. Redefining here as they are required to compile HttpThread.cpp.
// Making them public in HTTP.Build.cs would be an engine change and this plugin needs to work with vanilla UE5. 
#define UE_HTTP_DEFAULT_MAX_CONCURRENT_REQUESTS INT_MAX //int DefaultMaxConcurrentRequests { get { return int.MaxValue; } }
#define UE_HTTP_SUPPORT_TO_INCREASE_MAX_REQUESTS_AT_RUNTIME 1 //(bPlatformSupportToIncreaseMaxRequestsAtRuntime ? "1" : "0")
#endif

#include "Curl/CurlHttp.cpp"
#include "Curl/CurlHttpManager.cpp"
#include "Curl/CurlHttpThread.cpp"
#include "HttpThread.cpp"
#if UE_VERSION_OLDER_THAN(5, 3, 0)
#else
#if UE_VERSION_OLDER_THAN(5, 5, 0)
#include "IHttpThreadedRequest.cpp"
#endif
#endif
#if UE_VERSION_OLDER_THAN(5, 4, 0)
#else
#include "GenericPlatform/HttpRequestCommon.cpp"
#endif
#endif
