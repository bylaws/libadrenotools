// SPDX-License-Identifier: BSD-2-Clause
// Copyright © 2021 Billy Laws

#include <string>
#include <string_view>
#include <sys/stat.h>
#include <dlfcn.h>
#include <android/api-level.h>
#include <android/log.h>
#include <android_linker_ns.h>
#include "hook/main_hook.h"
#include <adrenotools/driver.h>

void *adrenotools_open_libvulkan(int dlopenMode, int featureFlags, const char *tmpLibDir, const char *hookLibDir, const char *customDriverDir, const char *customDriverName, const char *fileRedirectDir) {
    // Bail out if linkernsbyapss failed to load, this probably means we're on api < 28
    if (!linkernsbypass_load_status())
        return nullptr;

    // Always use memfd on Q+ since it's guaranteed to work
    if (android_get_device_api_level() >= 29)
        tmpLibDir = nullptr;

    // Verify that params for specific features are only passed if they are enabled
    if (!(featureFlags & ADRENOTOOLS_DRIVER_FILE_REDIRECT) && fileRedirectDir)
        return nullptr;

    if (!(featureFlags & ADRENOTOOLS_DRIVER_CUSTOM) && (customDriverDir || customDriverName))
        return nullptr;

    // Verify that params for enabled features are correct
    struct stat buf{};

    if (featureFlags & ADRENOTOOLS_DRIVER_CUSTOM) {
        if (!customDriverName || !customDriverDir)
            return nullptr;

        if (stat((std::string(customDriverDir) + customDriverName).c_str(), &buf) != 0)
            return nullptr;
    }

    // Verify that params for enabled features are correct
    if (featureFlags & ADRENOTOOLS_DRIVER_FILE_REDIRECT) {
        if (!fileRedirectDir)
            return nullptr;

        if (stat(fileRedirectDir, &buf) != 0)
            return nullptr;
    }

    // Will be destroyed by the libmain_hook.so destructor on unload
    auto *hookParam{new MainHookParam(featureFlags, tmpLibDir, hookLibDir, customDriverDir, customDriverName, fileRedirectDir)};

    return linkernsbypass_dlopen_unique_hooked("/system/lib64/libvulkan.so", tmpLibDir, dlopenMode, hookLibDir, "libmain_hook.so", nullptr, true, reinterpret_cast<void *>(hookParam));
}
