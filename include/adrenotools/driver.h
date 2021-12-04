// SPDX-License-Identifier: BSD-2-Clause
// Copyright © 2021 Billy Laws

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "priv.h"

/**
 * @brief Opens a new libvulkan.so instance according to `flags`
 * @param dlopenMode The dlopen mode to use when opening libvulkan
 * @param featureFlags Which adrenotools driver features to enable
 * @param tmpLibDir A writable directory to hold patched libraries, only used on api < 29 due to the lack of memfd support. If nullptr is passed and the API version is < 29 memfd usage will be attempted and if unsupported nullptr will be returned
 * @param hookLibDir The directory holding the built hooks
 * @param customDriverDir The directory to load a custom GPU driver named according to `customDriverName` from. Only used if ADRENOTOOLS_DRIVER_CUSTOM is set in `featureFlags`
 * @param customDriverName The soname of the custom driver to load. Only used if ADRENOTOOLS_DRIVER_CUSTOM is set in `featureFlags`
 * @param fileRedirectDir The directory which to redirect all file accesses performed by the driver to. Only used if ADRENOTOOLS_DRIVER_FILE_REDIRECT is set in `featureFlags`
 */
void *adrenotools_open_libvulkan(int dlopenMode, int featureFlags, const char *tmpLibDir, const char *hookLibDir, const char *customDriverDir, const char *customDriverName, const char *fileRedirectDir);

#ifdef __cplusplus
}
#endif
