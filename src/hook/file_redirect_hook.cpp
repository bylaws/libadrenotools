#include <string>
#include <cstdio>
#include <dlfcn.h>
#include <android/log.h>

#define TAG "file_redirect_hook"
#define LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO, TAG, fmt, ##__VA_ARGS__)

static decltype(&fopen) libc_fopen{[]() {
    auto libcHandle{dlopen("libc.so", RTLD_LAZY)};
    if (!libcHandle)
        __builtin_trap();

    auto sym{dlsym(libcHandle, "fopen")};
    if (!sym)
        __builtin_trap();

    return reinterpret_cast<decltype(&fopen)>(sym);
}()};

extern "C" {
    __attribute__((visibility("default"))) const char *hook_param; //!< The prefix to add to filesystem calls

    __attribute__((visibility("default"))) FILE *fopen(const char *filename, const char *mode) {
        if ((strncmp("/proc", filename, 5) == 0) || (strncmp("/sys", filename, 4) == 0)) {
            LOGI("fopen: passthrough: %s", filename);
            return libc_fopen(filename, mode);
        }

        auto replacement{std::string{hook_param} + filename};
        LOGI("fopen: %s -> %s", filename, replacement.c_str());

        return libc_fopen(replacement.c_str(), mode);
    }
}
