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

#include <android/hidl/base/1.0/BpHwBase.h>
#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <log/log.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

#define SHIM_MODE 1

#include "ext/common.h"
#include "ext/gr_utils.h"
#include "ext/mapper.h"
#include "ext/mapperextensions.h"

using namespace android::hardware;
using namespace android;

bool IBase::isRemote() const {
  return false;
}

namespace android::hardware::graphics::mapper {
namespace V2_0 {

sp<IMapper> IMapper::getService(const std::string& /* serviceName */, const bool /* getStub */) {
  return nullptr;
}

}  // namespace V2_0
namespace V3_0 {

sp<IMapper> IMapper::getService(const std::string& /* serviceName */, const bool /* getStub */) {
  return nullptr;
}

}  // namespace V3_0
namespace V4_0 {

sp<IMapper> IMapper::getService(const std::string& /* serviceName */, const bool /* getStub */) {
  return nullptr;
}

}  // namespace V4_0
}  // namespace android::hardware::graphics::mapper
namespace vendor::qti::hardware::display {
namespace mapper {
namespace V4_0 {

Return<sp<IQtiMapper>> IQtiMapper::castFrom(const sp<graphics::mapper::V4_0::IMapper>& /* parent */,
                                            bool /* emitError */) {
  return nullptr;
}

}  // namespace V4_0
namespace V3_0 {

Return<sp<IQtiMapper>> IQtiMapper::castFrom(const sp<graphics::mapper::V3_0::IMapper>& /* parent */,
                                            bool /* emitError */) {
  return nullptr;
}

}  // namespace V3_0
namespace V2_0 {

Return<sp<IQtiMapper>> IQtiMapper::castFrom(const sp<graphics::mapper::V2_0::IMapper>& /* parent */,
                                            bool /* emitError */) {
  return new IQtiMapper();
}

Return<void> IQtiMapper::getMapperExtensions(getMapperExtensions_cb _hidl_cb) {
  sp<mapperextensions::V1_0::IQtiMapperExtensions> ext{
      new mapperextensions::V1_0::IQtiMapperExtensions()};
  _hidl_cb(graphics::mapper::V2_0::Error::NONE, ext);
  ALOGD("IQtiMapperExtensions shim active");
  return Void();
}

}  // namespace V2_0
}  // namespace mapper
namespace mapperextensions {
namespace V1_1 {

IQtiMapperExtensions::IQtiMapperExtensions() {
  hw_module_t const* module;
  if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module)) {
    ALOGE("Failed to open gralloc module!");
    std::terminate();
  }

  if (gralloc1_open(module, &gralloc)) {
    ALOGE("Failed to open gralloc HAL!");
    std::terminate();
  }

  perform = reinterpret_cast<GRALLOC1_PFN_PERFORM>(
      gralloc->getFunction(gralloc, GRALLOC1_FUNCTION_PERFORM));
  if (!perform) {
    ALOGE("Failed to get gralloc perform helper!");
    std::terminate();
  }

  ALOGD("Opened gralloc1 HAL");
}

Return<sp<IQtiMapperExtensions>> IQtiMapperExtensions::castFrom(
    android::sp<V1_0::IQtiMapperExtensions> const& /* parent */, bool /* emitError */) {
  return new IQtiMapperExtensions();
}

Return<void> IQtiMapperExtensions::getRgbDataAddress(void* buffer, getRgbDataAddress_cb _hidl_cb) {
  void* rgb_data = nullptr;

  auto err = perform(gralloc, GRALLOC_MODULE_PERFORM_GET_RGB_DATA_ADDRESS, buffer, &rgb_data)
                 ? Error::BAD_BUFFER
                 : Error::NONE;
  _hidl_cb(err, rgb_data);

  ALOGE("%s: err: %d rgb_data: %p", __func__, err, rgb_data);
  return Void();
}

Return<void> IQtiMapperExtensions::getInterlacedFlag(void* buffer, getInterlacedFlag_cb _hidl_cb) {
  int flag = 0;
  auto perform = reinterpret_cast<GRALLOC1_PFN_PERFORM>(
      gralloc->getFunction(gralloc, GRALLOC1_FUNCTION_PERFORM));
  auto err = perform(gralloc, GRALLOC1_MODULE_PERFORM_GET_INTERLACE_FLAG, buffer, &flag)
                 ? Error::BAD_BUFFER
                 : Error::NONE;
  _hidl_cb(err, flag);
  ALOGE("%s: err: %d flag: %d", __func__, err, flag);
  return Void();
}
Return<void> IQtiMapperExtensions::getFd(void* buffer, getFd_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  int fd = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    fd = hnd->fd;
  }
  hidl_cb(err, fd);
  ALOGE("%s: fd: %d", __func__, fd);
  return Void();
}

Return<void> IQtiMapperExtensions::getWidth(void* buffer, getWidth_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  int width = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    width = hnd->width;
  }
  hidl_cb(err, width);
  ALOGE("%s: width: %d", __func__, width);
  return Void();
}

Return<void> IQtiMapperExtensions::getHeight(void* buffer, getHeight_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  int height = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    height = hnd->height;
  }
  hidl_cb(err, height);
  ALOGE("%s: height: %d", __func__, height);
  return Void();
}

Return<void> IQtiMapperExtensions::getFormat(void* buffer, getFormat_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  int format = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    format = hnd->format;
  }
  hidl_cb(err, format);

  ALOGE("%s: format: %d", __func__, format);
  return Void();
}

Return<void> IQtiMapperExtensions::getPrivateFlags(void* buffer, getPrivateFlags_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  int flags = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    flags = hnd->flags;
  }
  hidl_cb(err, flags);
  ALOGE("%s: flags: %d", __func__, flags);
  return Void();
}

Return<void> IQtiMapperExtensions::getUnalignedWidth(void* buffer, getUnalignedWidth_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  int unaligned_width = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    unaligned_width = hnd->unaligned_width;
  }
  hidl_cb(err, unaligned_width);

  ALOGE("%s: unaligned_width: %u", __func__, unaligned_width);
  return Void();
}

Return<void> IQtiMapperExtensions::getUnalignedHeight(void* buffer, getUnalignedHeight_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  int unaligned_height = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    unaligned_height = hnd->unaligned_height;
  }
  hidl_cb(err, unaligned_height);
  ALOGE("%s: unaligned_height: %u", __func__, unaligned_height);
  return Void();
}

Return<void> IQtiMapperExtensions::getLayerCount(void* buffer, getLayerCount_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  unsigned int layer_count = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    layer_count = hnd->layer_count;
  }
  hidl_cb(err, layer_count);
  ALOGE("%s: layer_count: %u", __func__, layer_count);
  return Void();
}

Return<void> IQtiMapperExtensions::getId(void* buffer, getId_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  uint64_t id = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    id = hnd->id;
  }
  hidl_cb(err, id);
  ALOGE("%s: id: %" PRIu64, __func__, id);
  return Void();
}

Return<void> IQtiMapperExtensions::getUsageFlags(void* buffer, getUsageFlags_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  uint64_t usage = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    usage = hnd->usage;
  }
  hidl_cb(err, usage);
  ALOGE("%s: usage: %" PRIu64, __func__, usage);

  return Void();
}

Return<void> IQtiMapperExtensions::getSize(void* buffer, getSize_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  unsigned int size = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    size = hnd->size;
  }
  hidl_cb(err, size);
  ALOGE("%s: size: %u", __func__, size);
  return Void();
}

Return<void> IQtiMapperExtensions::getOffset(void* buffer, getOffset_cb hidl_cb) {
  auto err = Error::BAD_BUFFER;
  unsigned int offset = 0;
  auto hnd = static_cast<private_handle_t*>(buffer);
  if (buffer != nullptr && private_handle_t::validate(hnd) == 0) {
    err = Error::NONE;
    offset = hnd->offset;
  }
  hidl_cb(err, offset);
  ALOGE("%s: offset: %u", __func__, offset);
  return Void();
}

Return<void> IQtiMapperExtensions::getFormatLayout(int32_t format, uint64_t usage, int32_t flags,
                                                   int32_t width, int32_t height,
                                                   getFormatLayout_cb _hidl_cb) {
  hidl_vec<PlaneLayout> plane_info;
  unsigned int alignedw = 0, alignedh = 0, tile_enabled = 0;
  int plane_count = 0;
  uint64_t size = 0;
  int custom_format = gralloc::GetImplDefinedFormat(usage, format);
  gralloc::BufferInfo info(width, height, custom_format, usage);

  auto err = perform(gralloc, GRALLOC_MODULE_PERFORM_GET_ATTRIBUTES, width, height, custom_format,
                     usage, usage, &alignedw, &alignedh, &tile_enabled)
                 ? Error::BAD_BUFFER
                 : Error::NONE;
  if (err != Error::NONE) {
    _hidl_cb(err, 0, plane_info);
    return Void();
  }

  size = gralloc::GetSize(info, alignedw, alignedh);
  gralloc::PlaneLayoutInfo plane_layout[8] = {};

  if (gralloc::IsUncompressedRGBFormat(custom_format) ||
      gralloc::IsCompressedRGBFormat(custom_format)) {
    gralloc::GetRGBPlaneInfo(info, custom_format, alignedw, alignedh, flags, &plane_count,
                             plane_layout);
  } else {
    ALOGE("%s: unsupported format %d", __func__, format);
    err = Error::BAD_BUFFER;
    _hidl_cb(err, size, plane_info);
    return Void();
  }

  plane_info.resize(plane_count);
  for (int i = 0; i < plane_count; i++) {
    plane_info[i].component = plane_layout[i].component;
    plane_info[i].h_subsampling = plane_layout[i].h_subsampling;
    plane_info[i].v_subsampling = plane_layout[i].v_subsampling;
    plane_info[i].offset = plane_layout[i].offset;
    plane_info[i].pixel_increment = plane_layout[i].step;
    plane_info[i].stride = plane_layout[i].stride;
    plane_info[i].stride_bytes = plane_layout[i].stride_bytes;
    plane_info[i].scanlines = plane_layout[i].scanlines;
    plane_info[i].size = plane_layout[i].size;
  }

  _hidl_cb(err, size, plane_info);
  return Void();
}

}  // namespace V1_1
}  // namespace mapperextensions
}  // namespace vendor::qti::hardware::display
