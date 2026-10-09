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
 * @brief    radar设备工厂
 * @version  V1.0.0
 */

#pragma once

#include <functional>
#include <map>

#include "device_base.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @brief  radar设备注册工厂
 */
class RadarDeviceFactory {
 public:
  using CONSTRUCT = std::function<RadarDevice*(const RadarCallBack& cb)>;

  template <typename Inherit>
  class Register_t {
   public:
    Register_t(const std::string& key) {
      RadarDeviceFactory::Instance().map_.emplace(
          key, [](const RadarCallBack& cb) {
            return new Inherit(cb);
          });
    }
  };
  /**
   * @brief      用于获取指定radar设备实例的unique指针
   * @param[in]  key 指定radar设备名
   * @retval     指定radar设备实例的unique指针
   */
  std::unique_ptr<RadarDevice> GetUnique(
      const std::string& key, const RadarCallBack& cb);
  /**
   * @brief      用于获取指定radar设备实例的shared指针
   * @param[in]  key 指定radar设备名
   * @retval     指定radar设备实例的shared指针
   */
  std::shared_ptr<RadarDevice> GetShared(
      const std::string& key, const RadarCallBack& cb);
  /**
   * @brief      用于获取radar设备工厂实例（单例）
   * @retval     radar设备工厂实例
   */
  static RadarDeviceFactory& Instance();

 private:
  RadarDevice* Produce(const std::string& key, const RadarCallBack& cb);

  RadarDeviceFactory(){};
  RadarDeviceFactory(const RadarDeviceFactory&) = delete;
  RadarDeviceFactory(RadarDeviceFactory&&)      = delete;

 private:
  std::map<std::string, CONSTRUCT> map_;
};

#define V2XOS_RADAR_REG(T) v2xos_reg_func_str_##T##_
/**
 * @brief      用于注册指定radar设备
 * @param[in]  T 具体radar设备子类型
 * @param[in]  key 注册的radar设备名字
 */
#define V2XOS_RADAR_REG_FACTORY(T, key) \
  static RadarDeviceFactory::Register_t<T> V2XOS_RADAR_REG(T)(key);

}  // namespace device
}  // namespace v2x
}  // namespace os
