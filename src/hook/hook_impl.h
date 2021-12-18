// SPDX-License-Identifier: BSD-2-Clause
// Copyright © 2021 Billy Laws

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <android/dlext.h>

void init_hook_param(const void *param);
	
void *hook_android_dlopen_ext(const char *filename, int flags, const android_dlextinfo *extinfo);

void *hook_android_load_sphal_library(const char *filename, int flags);

FILE *hook_fopen(const char *filename, const char *mode);


#ifdef __cplusplus
}
#endif

