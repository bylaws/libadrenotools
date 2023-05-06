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
 * @param userMappingHandle A pointer to a void* which will be set to the mapping handle if ADRENOTOOLS_DRIVER_GPU_MAPPING_IMPORT is set in `featureFlags`
 */
void *adrenotools_open_libvulkan(int dlopenMode, int featureFlags, const char *tmpLibDir, const char *hookLibDir, const char *customDriverDir, const char *customDriverName, const char *fileRedirectDir, void **userMappingHandle);

/**
 * @brief Imports the given CPU mapped memory range into the GSL allocator. The out_mapping should have been passed to adrenotools_open_libvulkan with ADRENOTOOLS_DRIVER_GPU_MAPPING_IMPORT set in `featureFlags`. This should then be followed by a call to vkAllocateMemory with a matching size which will return a VkDeviceMemory view over the input region
 * @param handle Mapping handle that was returned by adrenotools_open_libvulkan
 * @param hostPtr The host pointer to import
 * @param size The size of the region to import
 * @return True on success
 */
bool adrenotools_import_user_mem(void *handle, void *hostPtr, uint64_t size);

/**
 * @note This function should be called after adrenotools_open_libvulkan and Vulkan driver init to check if the mapping import hook loaded successfully
 * @return True if the mapping was successfully imported (or initialization succeeded)
 */
bool adrenotools_validate_gpu_mapping(void *handle);

/**
 * @brief Provides a way to force the GPU to run at the maximum possible clocks (thermal constraints will still be applied)
 */
void adrenotools_set_turbo(bool turbo);

#ifdef __cplusplus
}
#endif
