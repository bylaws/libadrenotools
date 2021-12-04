#include <cstring>
#include <android/dlext.h>
#include <android_linker_ns.h>
#include <android/log.h>
#include "main_hook.h"

#define TAG "driver_load_hook"
#define LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO, TAG, fmt, ##__VA_ARGS__)

extern "C" {
    // The namespace that should be used to load vulkan drivers, set dlopen_hooked from the hookParam argument
    __attribute__((visibility("default"))) const MainHookParam *hook_param;

    __attribute__((destructor)) static void free_hook_param() {
        delete hook_param;
    }

    __attribute__((visibility("default"))) void *android_dlopen_ext(const char *filename, int flags, const android_dlextinfo *extinfo) {
        auto fallback{[&]() {
            LOGI("android_dlopen_ext: falling back!");
            return libdl_android_dlopen_ext(filename, flags, extinfo);
        }};

        LOGI("android_dlopen_ext: filename: %s", filename);

        // Ignore non-vulkan libraries
        if (!strstr(filename, "vulkan."))
            return libdl_android_dlopen_ext(filename, flags, extinfo);

        if (extinfo->library_namespace == nullptr || !(extinfo->flags & ANDROID_DLEXT_USE_NAMESPACE)) {
            LOGI("android_dlopen_ext: hook failed: namespace not supplied!");
            return fallback();
        }

        // customDriverDir will be empty if ADRENOTOOLS_DRIVER_CUSTOM isn't set therefore it's fine to have either way
        auto driverNs{android_create_namespace(filename, hook_param->customDriverDir.c_str(),
                                               hook_param->hookLibDir.c_str(), ANDROID_NAMESPACE_TYPE_SHARED,
                                               nullptr, extinfo->library_namespace)};
        if (!driverNs) {
            LOGI("android_dlopen_ext: hook failed: namespace not supplied!");
            return fallback();
        }

        // Hook libs depend on libandroid which is unlikely to be in the supplied driver namespace so we have to link it over
        android_link_namespaces(driverNs, nullptr, "libandroid.so");

        if (hook_param->featureFlags & ADRENOTOOLS_DRIVER_FILE_REDIRECT) {
            if (!linkernsbypass_namespace_apply_hook("libfile_redirect_hook.so", driverNs,
                                                     reinterpret_cast<const void *>(hook_param->fileRedirectDir.c_str()))) {
                LOGI("android_dlopen_ext: hook failed: failed to apply libfopen_redirect_hook!");
                return fallback();
            }

            LOGI("android_dlopen_ext: applied libfopen_redirect_hook");
        }

        // Use our new namespace to load the vulkan driver
        auto newExtinfo{*extinfo};
        newExtinfo.library_namespace = driverNs;

        // TODO: If there is already an instance of a vulkan driver loaded hooks won't be applied, this will only be the case for skiavk generally
        // To fix this we would need to search /proc/self/maps for the file to a loaded instance of the library in order to read it to patch the soname and load it uniquely
        if (hook_param->featureFlags & ADRENOTOOLS_DRIVER_CUSTOM) {
            LOGI("android_dlopen_ext: loading custom driver: %s%s", hook_param->customDriverDir.c_str(), hook_param->customDriverName.c_str());
            return libdl_android_dlopen_ext(hook_param->customDriverName.c_str(), flags, &newExtinfo);
        } else {
            LOGI("android_dlopen_ext: loading default driver: %s", filename);
            return libdl_android_dlopen_ext(filename, flags, &newExtinfo);
        }
    }

    __attribute__((visibility("default"))) void *android_load_sphal_library(const char *filename, int flags) {
        // https://android.googlesource.com/platform/system/core/+/master/libvndksupport/linker.cpp
        for (const char *name : {"sphal", "vendor", "default"}) {
            if (auto vendorNs{android_get_exported_namespace(name)}) {
                android_dlextinfo dlextinfo{
                    .flags = ANDROID_DLEXT_USE_NAMESPACE,
                    .library_namespace = vendorNs,
                };

                return android_dlopen_ext(filename, flags, &dlextinfo);
            }
        }

        return nullptr;
    }
}

