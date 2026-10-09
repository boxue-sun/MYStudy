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
 * @brief    cloud设备工厂
 * @version  V1.0.0
 */

#pragma once

#include <functional>
#include <map>

#include "base/device_connect/cloud/device_base.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @brief  Cloud设备注册工厂
 */
class CloudDeviceFactory {
 public:
  using CONSTRUCT = std::function<CloudDevice*(const CloudCallBack& cb)>;

  template <typename Inherit>
  class Register_t {
   public:
    Register_t(const std::string& key) {
      CloudDeviceFactory::Instance().map_.emplace(
          key, [](const CloudCallBack& cb) {
            return new Inherit(cb);
          });
    }
  };
  /**
   * @brief      用于获取指定Cloud连接协议实例的unique指针
   * @param[in]  key 指定Cloud连接协议名
   * @retval     指定Cloud连接协议实例的unique指针
   */
  std::unique_ptr<CloudDevice> GetUnique(
      const std::string& key, const CloudCallBack& cb);
  /**
   * @brief      用于获取指定Cloud连接协议实例的shared指针
   * @param[in]  key 指定Cloud连接协议名
   * @retval     指定Cloud连接协议实例的shared指针
   */
  std::shared_ptr<CloudDevice> GetShared(
      const std::string& key, const CloudCallBack& cb);
  /**
   * @brief      用于获取Cloud连接协议工厂实例（单例）
   * @retval     Cloud连接协议工厂实例
   */
  static CloudDeviceFactory& Instance();

 private:
  CloudDevice* Produce(const std::string& key, const CloudCallBack& cb);

  CloudDeviceFactory(){};
  CloudDeviceFactory(const CloudDeviceFactory&) = delete;
  CloudDeviceFactory(CloudDeviceFactory&&)      = delete;

 private:
  std::map<std::string, CONSTRUCT> map_;
};

#define V2XOS_CLOUD_REG(T) v2xos_reg_func_str_##T##_
/**
 * @brief      用于注册指定Cloud连接协议
 * @param[in]  T 具体Cloud连接协议子类型
 * @param[in]  key 注册的Cloud连接协议名字
 */
#define V2XOS_CLOUD_REG_FACTORY(T, key) \
  static CloudDeviceFactory::Register_t<T> V2XOS_CLOUD_REG(T)(key);

}  // namespace device
}  // namespace v2x
}  // namespace os
