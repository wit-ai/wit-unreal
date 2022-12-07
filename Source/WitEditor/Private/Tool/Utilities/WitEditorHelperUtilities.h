/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"
#if UE_VERSION_OLDER_THAN(5,1,0)
#include "EditorStyleSet.h"
#endif

/**
 * Helper class
 */
class WITEDITOR_API WitEditorHelperUtilities
{
public:

	static const FSlateBrush* GetBrush( FName PropertyName, const ANSICHAR* Specifier = nullptr )
	{
#if UE_VERSION_OLDER_THAN(5,1,0)
		return FEditorStyle::GetBrush(PropertyName, Specifier);
#else
		return FAppStyle::GetBrush(PropertyName, Specifier);
#endif
	}
};
