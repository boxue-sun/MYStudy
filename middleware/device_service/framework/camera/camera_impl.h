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

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <thread>

#include "base/common/msg_queue.h"
#include "base/device_connect/camera/camera_base.h"

namespace airos {
namespace middleware {
namespace device_service {

class CameraImpl {
 public:
  using CameraImageData = base::device::CameraImageData;
  using CameraDevice    = base::device::CameraDevice;
  using ImgQueue =
      std::shared_ptr<base::common::MsgQueue<std::shared_ptr<CameraImageData>>>;

  bool Init(const base::device::CameraInitConfig& config);
  std::shared_ptr<CameraImageData> GetCameraData(
      unsigned int timeout_ms = 0);  // 0 means block

  CameraImpl()                  = default;
  CameraImpl(const CameraImpl&) = delete;
  CameraImpl(CameraImpl&&)      = delete;
  ~CameraImpl();

 private:
  void ReceiveCallBack(const std::shared_ptr<CameraImageData>& data);

 private:
  std::string camera_name_;
  size_t cache_num_ = 3;
  int time_out_ms_  = 100;
  ImgQueue img_queue_;
  std::shared_ptr<CameraDevice> device_;
};

}  // namespace device_service
}  // namespace middleware
}  // namespace airos
