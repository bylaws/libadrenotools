#include <initializer_list>
#include <string>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <android_linker_ns.h>
#include <android/dlext.h>
#include <android/log.h>
#include "hook_impl_params.h"
#include "hook_impl.h"

#define TAG "hook_impl"
#define LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO, TAG, fmt, ##__VA_ARGS__)

const HookImplParams *hook_params; //!< Bunch of info needed to load/patch the driver

__attribute__((visibility("default"))) void init_hook_param(const void *param) {
    hook_params = reinterpret_cast<const HookImplParams *>(param);
}

__attribute__((visibility("default"))) void *hook_android_dlopen_ext(const char *filename, int flags, const android_dlextinfo *extinfo) {
    auto fallback{[&]() {
        LOGI("hook_android_dlopen_ext: falling back!");
        return android_dlopen_ext(filename, flags, extinfo);
    }};

    LOGI("hook_android_dlopen_ext: filename: %s", filename);

    // Ignore non-vulkan libraries
    if (!strstr(filename, "vulkan."))
        return android_dlopen_ext(filename, flags, extinfo);

    if (extinfo->library_namespace == nullptr || !(extinfo->flags & ANDROID_DLEXT_USE_NAMESPACE)) {
        LOGI("hook_android_dlopen_ext: hook failed: namespace not supplied!");
        return fallback();
    }

    // customDriverDir will be empty if ADRENOTOOLS_DRIVER_CUSTOM isn't set therefore it's fine to have either way
    auto driverNs{android_create_namespace(filename, hook_params->customDriverDir.c_str(),
                                           hook_params->hookLibDir.c_str(), ANDROID_NAMESPACE_TYPE_SHARED,
                                           nullptr, extinfo->library_namespace)};
    if (!driverNs) {
        LOGI("hook_android_dlopen_ext: hook failed: namespace not supplied!");
        return fallback();
    }

    // We depend on libandroid which is unlikely to be in the supplied driver namespace so we have to link it over
    android_link_namespaces(driverNs, nullptr, "libandroid.so");
    
    // Preload ourself, a new instance will be created since we have different linker ancestory
    // If we don't preload we get a weird issue where despite being in NEEDED of the hook lib the hook's symbols will overwrite ours and cause an infinite loop
    auto hookImpl{linkernsbypass_namespace_dlopen("libhook_impl.so", RTLD_NOW, driverNs)};
    if (!hookImpl)
        return nullptr;

    // Pass parameters to ourself
    auto initHookParam{reinterpret_cast<void (*)(const void *)>(dlsym(hookImpl, "init_hook_param"))};
    if (!initHookParam)
        return nullptr;
    
    initHookParam(hook_params);

    if (hook_params->featureFlags & ADRENOTOOLS_DRIVER_FILE_REDIRECT) {
        if (!linkernsbypass_namespace_dlopen("libfile_redirect_hook.so", RTLD_GLOBAL, driverNs)) {
            LOGI("hook_android_dlopen_ext: hook failed: failed to apply libfopen_redirect_hook!");
            return fallback();
        }

        LOGI("hook_android_dlopen_ext: applied libfile_redirect_hook");
    }

    // Use our new namespace to load the vulkan driver
    auto newExtinfo{*extinfo};
    newExtinfo.library_namespace = driverNs;

    // TODO: If there is already an instance of a vulkan driver loaded hooks won't be applied, this will only be the case for skiavk generally
    // To fix this we would need to search /proc/self/maps for the file to a loaded instance of the library in order to read it to patch the soname and load it uniquely
    if (hook_params->featureFlags & ADRENOTOOLS_DRIVER_CUSTOM) {
        LOGI("hook_android_dlopen_ext: loading custom driver: %s%s", hook_params->customDriverDir.c_str(), hook_params->customDriverName.c_str());
        return android_dlopen_ext(hook_params->customDriverName.c_str(), flags, &newExtinfo);
    } else {
        LOGI("hook_android_dlopen_ext: loading default driver: %s", filename);
        return android_dlopen_ext(filename, flags, &newExtinfo);
    }
}

__attribute__((visibility("default"))) void *hook_android_load_sphal_library(const char *filename, int flags) {
    LOGI("hook_android_load_sphal_library: filename: %s", filename);
    
    // https://android.googlesource.com/platform/system/core/+/master/libvndksupport/linker.cpp
    for (const char *name : {"sphal", "vendor", "default"}) {
        if (auto vendorNs{android_get_exported_namespace(name)}) {
            android_dlextinfo dlextinfo{
                .flags = ANDROID_DLEXT_USE_NAMESPACE,
                .library_namespace = vendorNs,
            };

            return hook_android_dlopen_ext(filename, flags, &dlextinfo);
        }
    }

    return nullptr;
}
    
__attribute__((visibility("default"))) FILE *hook_fopen(const char *filename, const char *mode) {
    if (!strncmp("/proc", filename, 5) || !strncmp("/sys", filename, 4)) {
        LOGI("hook_fopen: passthrough: %s", filename);
        return fopen(filename, mode);
    }

    auto replacement{hook_params->fileRedirectDir + filename};
    LOGI("hook_fopen: %s -> %s", filename, replacement.c_str());

    return fopen(replacement.c_str(), mode);
}
