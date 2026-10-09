/******************************************************************************
 * Copyright 2022 The Airos Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#pragma once

#ifndef __cplusplus
#error "This is a c++ header!"
#endif

#include <sys/time.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "base/cv_image/image.h"

namespace airos {
namespace base {
namespace device {

enum GPUImageMode {
  GPU_IMAGE_MODE_RGB24 = 0,
  GPU_IMAGE_MODE_BGR24,
};

class GPUImage {
 public:
  int dev_id                         = 0;
  int width                          = 0;
  int height                         = 0;
  int64_t meatime_us                 = 0;
  int64_t yuvtime_us                 = 0;
  airos::base::Color mode            = airos::base::Color::RGB;
  std::shared_ptr<void> gpu_ptr      = nullptr;
  uint32_t sequence_num              = 0;
  std::string channel_str            = "";
  int64_t single_camera_hik_sequence = 0;
  int64_t single_camera_sequence     = 0;
  std::shared_ptr<void> ori_gpu_ptr  = nullptr;
  int ori_width                      = 0;
  int ori_height                     = 0;
};

// // 相机发送数据包
// struct HKCameraData {
//     std::shared_ptr<void> image;  // 注意：
//     需要相机来管理shared_ptr的析构函数 std::string camera_name; uint32_t
//     device_id; GPUImageMode mode; uint32_t height; uint32_t width; uint64_t
//     timestamp;  // nano second uint32_t sequence_num;
// };

typedef std::unordered_map<std::string, std::shared_ptr<GPUImage>> GPUImageMap;
// 相机设备输出回调函数类型
// using HKCameraDeviceCallBack =
//     std::function<void(const std::shared_ptr<HKCameraData>)>;

}  // END namespace device
}  // END namespace base
}  // namespace airos
