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
 * @brief    rsu设备工厂
 * @version  V1.0.0
 */

#pragma once

#include <functional>
#include <map>

#include "base/device_connect/rsu/device_base.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @brief  rsu设备注册工厂
 */
class RSUDeviceFactory {
 public:
  using CONSTRUCT = std::function<RSUDevice*(const RSUCallBack& cb)>;

  template <typename Inherit>
  class Register_t {
   public:
    Register_t(const std::string& key) {
      RSUDeviceFactory::Instance().map_.emplace(key, [](const RSUCallBack& cb) {
        return new Inherit(cb);
      });
    }
  };
  /**
   * @brief      用于获取指定rsu设备实例的unique指针
   * @param[in]  key 指定rsu设备名
   * @retval     指定rsu设备实例的unique指针
   */
  std::unique_ptr<RSUDevice> GetUnique(
      const std::string& key, const RSUCallBack& cb);
  /**
   * @brief      用于获取指定rsu设备实例的shared指针
   * @param[in]  key 指定rsu设备名
   * @retval     指定rsu设备实例的shared指针
   */
  std::shared_ptr<RSUDevice> GetShared(
      const std::string& key, const RSUCallBack& cb);
  /**
   * @brief      用于获取rsu设备工厂实例（单例）
   * @retval     rsu设备工厂实例
   */
  static RSUDeviceFactory& Instance();

 private:
  RSUDevice* Produce(const std::string& key, const RSUCallBack& cb);

  RSUDeviceFactory(){};
  RSUDeviceFactory(const RSUDeviceFactory&) = delete;
  RSUDeviceFactory(RSUDeviceFactory&&)      = delete;

 private:
  std::map<std::string, CONSTRUCT> map_;
};

#define V2XOS_RSU_REG(T) v2xos_reg_func_str_##T##_
/**
 * @brief      用于注册指定rsu设备
 * @param[in]  T 具体rsu设备子类型
 * @param[in]  key 注册的rsu设备名字
 */
#define V2XOS_RSU_REG_FACTORY(T, key) \
  static RSUDeviceFactory::Register_t<T> V2XOS_RSU_REG(T)(key);

}  // namespace device
}  // namespace v2x
}  // namespace os
