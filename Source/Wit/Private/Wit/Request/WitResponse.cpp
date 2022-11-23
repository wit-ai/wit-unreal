/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Wit/Request/WitResponse.h"

/**
 * Reset the response to its defaults
 */
void FWitResponse::Reset()
{
	Text.Empty(0);
	Intents.Empty(0);
	Entities.Empty(0);
	AllEntities.Empty(0);
	Traits.Empty(0);
	Is_Final = false;
}
