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
 * @file     device_factory.h
 * @brief    radar设备工厂实现
 * @version  V1.0.0
 */

#include "base/device_connect/radar/device_factory.h"

namespace os {
namespace v2x {
namespace device {

RadarDeviceFactory& RadarDeviceFactory::Instance() {
  static RadarDeviceFactory instance_;
  return instance_;
}

std::unique_ptr<RadarDevice> RadarDeviceFactory::GetUnique(
    const std::string& key, const RadarCallBack& cb) {
  return std::unique_ptr<RadarDevice>(Produce(key, cb));
}

std::shared_ptr<RadarDevice> RadarDeviceFactory::GetShared(
    const std::string& key, const RadarCallBack& cb) {
  return std::shared_ptr<RadarDevice>(Produce(key, cb));
}

RadarDevice* RadarDeviceFactory::Produce(
    const std::string& key, const RadarCallBack& cb) {
  if (map_.find(key) == map_.end()) {
    return nullptr;
  }
  return map_[key](cb);
}

}  // namespace device
}  // namespace v2x
}  // namespace os
