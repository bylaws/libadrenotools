/*
 * Copyright (c) 2016-2022, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "qtimapper-shim"

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>
#include <android/hidl/base/1.0/BpHwBase.h>
#include <log/log.h>
#include "ext_headers/common.h"
#include "ext_headers/mapper.h"
#include "ext_headers/mapperextensions.h"

using namespace android::hardware;
using namespace android;

bool IBase::isRemote() const {
    return false;
}

namespace android::hardware::graphics::mapper {
namespace V2_0 {

sp<IMapper> IMapper::getService(const std::string &/* serviceName */, const bool /* getStub */) {
    return nullptr;
}

}
namespace V3_0 {

sp<IMapper> IMapper::getService(const std::string &/* serviceName */, const bool /* getStub */) {
    return nullptr;
}

}
namespace V4_0 {

sp<IMapper> IMapper::getService(const std::string &/* serviceName */, const bool /* getStub */) {
    return nullptr;
}

}
}
namespace vendor::qti::hardware::display {
namespace mapper {
namespace V4_0 {

Return<sp<IQtiMapper>> IQtiMapper::castFrom(const sp<graphics::mapper::V4_0::IMapper> &/* parent */, bool /* emitV1_0::Error */) {
    return nullptr;
}

}
namespace V3_0 {

Return<sp<IQtiMapper>> IQtiMapper::castFrom(const sp<graphics::mapper::V3_0::IMapper> &/* parent */, bool /* emitV1_0::Error */) {
    return nullptr;
}

}
namespace V2_0 {

Return<sp<IQtiMapper>> IQtiMapper::castFrom(const sp<graphics::mapper::V2_0::IMapper> &/* parent */, bool /* emitV1_0::Error */) {
    return new IQtiMapper();
}

Return<void> IQtiMapper::getMapperExtensions(getMapperExtensions_cb _hidl_cb) {
    sp<mapperextensions::V1_0::IQtiMapperExtensions> ext{new mapperextensions::V1_0::IQtiMapperExtensions()};
    _hidl_cb(graphics::mapper::V2_0::Error::NONE, ext);
    ALOGD("IQtiMapperExtensions shim active");
    return Void();
}

}
}
namespace mapperextensions {
namespace V1_1 {

IQtiMapperExtensions::IQtiMapperExtensions() {
    hw_module_t const *module;
    if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module)) {
        ALOGE("Failed to open gralloc module!");
        std::terminate();
    }

   	if (gralloc1_open(module, &gralloc)) {
        ALOGE("Failed to open gralloc HAL!");
        std::terminate();
    }

    perform = reinterpret_cast<GRALLOC1_PFN_PERFORM>(gralloc->getFunction(gralloc, GRALLOC1_FUNCTION_PERFORM));
    if (!perform) {
        ALOGE("Failed to get gralloc perform helper!");
        std::terminate();
    }

    ALOGD("Opened gralloc1 HAL");
}


Return<sp<IQtiMapperExtensions>> IQtiMapperExtensions::castFrom(android::sp<V1_0::IQtiMapperExtensions> const& /* parent */, bool /* emitV1_0::Error */) {
    return new IQtiMapperExtensions();
}

Return<void> IQtiMapperExtensions::getRgbDataAddress(void *buffer, getRgbDataAddress_cb _hidl_cb) {
    void *rgb_data = nullptr;
    auto err = perform(gralloc, GRALLOC_MODULE_PERFORM_GET_RGB_DATA_ADDRESS, buffer, &rgb_data) ? V1_0::Error::NONE : V1_0::Error::BAD_BUFFER;
    _hidl_cb(err, rgb_data);
    return Void();
}

Return<void> IQtiMapperExtensions::getInterlacedFlag(void* buffer, getInterlacedFlag_cb _hidl_cb) {
    int flag = 0;
    auto perform = reinterpret_cast<GRALLOC1_PFN_PERFORM>(gralloc->getFunction(gralloc, GRALLOC1_FUNCTION_PERFORM));
    auto err = perform(gralloc, GRALLOC1_MODULE_PERFORM_GET_INTERLACE_FLAG, buffer, &flag) ? V1_0::Error::NONE : V1_0::Error::BAD_BUFFER;
    _hidl_cb(err, flag);
    return Void();
}
Return<void> IQtiMapperExtensions::getFd(void *buffer, getFd_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  int fd = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    fd = hnd->fd;
  }
  hidl_cb(err, fd);
  return Void();
}

Return<void> IQtiMapperExtensions::getWidth(void *buffer, getWidth_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  int width = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    width = hnd->width;
  }
  hidl_cb(err, width);
  return Void();
}

Return<void> IQtiMapperExtensions::getHeight(void *buffer, getHeight_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  int height = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    height = hnd->height;
  }
  hidl_cb(err, height);
  return Void();
}

Return<void> IQtiMapperExtensions::getFormat(void *buffer, getFormat_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  int format = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    format = hnd->format;
  }
  hidl_cb(err, format);
  return Void();
}

Return<void> IQtiMapperExtensions::getPrivateFlags(void *buffer, getPrivateFlags_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  int flags = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    flags = hnd->flags;
  }
  hidl_cb(err, flags);
  return Void();
}

Return<void> IQtiMapperExtensions::getUnalignedWidth(void *buffer, getUnalignedWidth_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  int unaligned_width = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    unaligned_width = hnd->unaligned_width;
  }
  hidl_cb(err, unaligned_width);
  return Void();
}

Return<void> IQtiMapperExtensions::getUnalignedHeight(void *buffer, getUnalignedHeight_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  int unaligned_height = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    unaligned_height = hnd->unaligned_height;
  }
  hidl_cb(err, unaligned_height);
  return Void();
}

Return<void> IQtiMapperExtensions::getLayerCount(void *buffer, getLayerCount_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  unsigned int layer_count = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    layer_count = hnd->layer_count;
  }
  hidl_cb(err, layer_count);
  return Void();
}

Return<void> IQtiMapperExtensions::getId(void *buffer, getId_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  uint64_t id = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    id = hnd->id;
  }
  hidl_cb(err, id);
  return Void();
}

Return<void> IQtiMapperExtensions::getUsageFlags(void *buffer, getUsageFlags_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  uint64_t usage = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    usage = hnd->usage;
  }
  hidl_cb(err, usage);
  return Void();
}

Return<void> IQtiMapperExtensions::getSize(void *buffer, getSize_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  unsigned int size = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    size = hnd->size;
  }
  hidl_cb(err, size);
  return Void();
}

Return<void> IQtiMapperExtensions::getOffset(void *buffer, getOffset_cb hidl_cb) {
  auto err = V1_0::Error::BAD_BUFFER;
  unsigned int offset = 0;
  auto hnd = static_cast<private_handle_t *>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = V1_0::Error::NONE;
    offset = hnd->offset;
  }
  hidl_cb(err, offset);
  return Void();
}

}
}
}
