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
 * @brief    lidar设备工厂实现
 * @version  V1.0.0
 */

#include "base/device_connect/lidar/device_factory.h"

namespace os {
namespace v2x {
namespace device {

LidarDeviceFactory& LidarDeviceFactory::Instance() {
  static LidarDeviceFactory instance_;
  return instance_;
}

std::unique_ptr<LidarDevice> LidarDeviceFactory::GetUnique(
    const std::string& key, const LidarCallBack& cb) {
  return std::unique_ptr<LidarDevice>(Produce(key, cb));
}

std::shared_ptr<LidarDevice> LidarDeviceFactory::GetShared(
    const std::string& key, const LidarCallBack& cb) {
  return std::shared_ptr<LidarDevice>(Produce(key, cb));
}

LidarDevice* LidarDeviceFactory::Produce(
    const std::string& key, const LidarCallBack& cb) {
  if (map_.find(key) == map_.end()) {
    return nullptr;
  }
  return map_[key](cb);
}

}  // namespace device
}  // namespace v2x
}  // namespace os
