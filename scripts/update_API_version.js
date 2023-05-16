/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

const updateFileAsync = require("./updateFileAsync");

const KEY_STRING_START = 'FString ApiVersion{"';
const KEY_STRING_END = '"';

function getLatestVersion() {
  let date_ob = new Date();
  let date = ("0" + date_ob.getDate()).slice(-2);
  let month = ("0" + (date_ob.getMonth() + 1)).slice(-2);
  let year = date_ob.getFullYear();
  return `${year}${month}${date}`
}

updateFileAsync('..\\Source\\Wit\\Public\\Wit\\Configuration\\WitAppConfiguration.h', KEY_STRING_START, getLatestVersion(), KEY_STRING_END);
