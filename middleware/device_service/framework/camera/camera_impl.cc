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

#include "middleware/device_service/framework/camera/camera_impl.h"

#include <chrono>
#include <functional>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <yaml-cpp/yaml.h>

#include "base/common/log.h"
#include "base/device_connect/camera/camera_factory.h"
#include "base/plugin/modules_loader/dynamic_loader.h"

namespace airos {
namespace middleware {
namespace device_service {

using CameraImageData = base::device::CameraImageData;
using CameraDevice    = base::device::CameraDevice;
using ImgQueue =
    std::shared_ptr<base::common::MsgQueue<std::shared_ptr<CameraImageData>>>;

bool CameraImpl::Init(const base::device::CameraInitConfig& config) {
  static const int cache_num = 1;
  img_queue_                 = std::make_shared<
                      base::common::MsgQueue<std::shared_ptr<CameraImageData>>>(cache_num);
  if (img_queue_ == nullptr) return false;
  airos::base::DynamicLoader::GetInstance().LoadDevicePackage(
      "device/lib/camera/");
  device_ = airos::base::device::CameraDeviceFactory::Instance().GetShared(
      config.camera_manufactor,
      std::bind(&CameraImpl::ReceiveCallBack, this, std::placeholders::_1));
  if (device_ == nullptr) {
    return false;
  }
  if (!device_->Init(config)) {
    return false;
  }
  img_queue_->enable();
  return true;
}

std::shared_ptr<CameraImageData> CameraImpl::GetCameraData(
    unsigned int timeout_ms) {
  std::shared_ptr<CameraImageData> res;
  if (img_queue_ == nullptr ||
      img_queue_->pop(&res, base::common::PopPolicy::blocking, timeout_ms) !=
          0) {
    LOG_ERROR << "queue empty";
    return nullptr;
  }
  return res;
}

void CameraImpl::ReceiveCallBack(const std::shared_ptr<CameraImageData>& data) {
  if (img_queue_) img_queue_->push(data, base::common::PushPolicy::discard_old);
  return;
}

CameraImpl::~CameraImpl() {
  if (img_queue_) img_queue_->disable();
}

}  // namespace device_service
}  // namespace middleware
}  // namespace airos
