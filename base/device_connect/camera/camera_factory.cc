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

/**
 * @file     device_factory.cpp
 * @brief    相机设备工厂实现
 * @version  V1.0.0
 */

#include "base/device_connect/camera/camera_factory.h"

#include <memory>
#include <string>

namespace airos {
namespace base {
namespace device {

CameraDeviceFactory& CameraDeviceFactory::Instance() {
  static CameraDeviceFactory instance_;
  return instance_;
}

std::unique_ptr<CameraDevice> CameraDeviceFactory::GetUnique(
    const std::string& key, const CameraImageCallBack& cb) {
  return std::unique_ptr<CameraDevice>(Produce(key, cb));
}

std::shared_ptr<CameraDevice> CameraDeviceFactory::GetShared(
    const std::string& key, const CameraImageCallBack& cb) {
  return std::shared_ptr<CameraDevice>(Produce(key, cb));
}

CameraDevice* CameraDeviceFactory::Produce(
    const std::string& key, const CameraImageCallBack& cb) {
  if (map_.find(key) == map_.end()) {
    return nullptr;
  }
  return map_[key](cb);
}

}  // namespace device
}  // namespace base
}  // namespace airos
