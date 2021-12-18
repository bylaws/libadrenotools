// SPDX-License-Identifier: BSD-2-Clause
// Copyright © 2021 Billy Laws

#include <string>
#include <string_view>
#include <sys/stat.h>
#include <dlfcn.h>
#include <android/api-level.h>
#include <android/log.h>
#include <android_linker_ns.h>
#include "hook/hook_impl_params.h"
#include <adrenotools/driver.h>

void *adrenotools_open_libvulkan(int dlopenFlags, int featureFlags, const char *tmpLibDir, const char *hookLibDir, const char *customDriverDir, const char *customDriverName, const char *fileRedirectDir) {
    // Bail out if linkernsbypass failed to load, this probably means we're on api < 28
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

    // Create a namespace that can isolate our hook from the classloader namespace
    auto hookNs{android_create_namespace("adrenotools-libvulkan", hookLibDir, nullptr, ANDROID_NAMESPACE_TYPE_SHARED, nullptr, nullptr)};

    // Link it to the default namespace so the hook can use libandroid etc
    if (!linkernsbypass_link_namespace_to_default_all_libs(hookNs))
        return nullptr;

    // Preload the hook implementation, otherwise we get a weird issue where despite being in NEEDED of the hook lib the hook's symbols will overwrite ours and cause an infinite loop
    auto hookImpl{linkernsbypass_namespace_dlopen("libhook_impl.so", RTLD_NOW, hookNs)};
    if (!hookImpl)
        return nullptr;

    // Pass parameters to the hook implementation
    auto initHookParam{reinterpret_cast<void (*)(const void *)>(dlsym(hookImpl, "init_hook_param"))};
    if (!initHookParam)
        return nullptr;

    initHookParam(new HookImplParams(featureFlags, tmpLibDir, hookLibDir, customDriverDir, customDriverName, fileRedirectDir));

    // Load the libvulkan hook into the isolated namespace
    if (!linkernsbypass_namespace_dlopen("libmain_hook.so", RTLD_GLOBAL, hookNs))
        return nullptr;

    return linkernsbypass_namespace_dlopen_unique("/system/lib64/libvulkan.so", tmpLibDir, dlopenFlags, hookNs);
}
