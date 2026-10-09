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
 * @brief    相机设备注册工厂
 * @version  V1.0.0
 */

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "base/device_connect/camera/camera_base.h"

namespace airos {
namespace base {
namespace device {

/**
 * @brief  相机设备注册工厂
 */
class CameraDeviceFactory {
 public:
  using CONSTRUCT = std::function<CameraDevice*(const CameraImageCallBack& cb)>;

  template <typename Inherit>
  class Register_t {
   public:
    explicit Register_t(const std::string& key) {
      CameraDeviceFactory::Instance().map_.emplace(
          key, [](const CameraImageCallBack& cb) {
            return new Inherit(cb);
          });
    }
  };
  /**
   * @brief      用于获取指定相机设备实例的unique指针
   * @param[in]  key 指定相机设备名
   * @retval     指定相机设备实例的unique指针
   */
  std::unique_ptr<CameraDevice> GetUnique(
      const std::string& key, const CameraImageCallBack& cb);
  /**
   * @brief      用于获取指定相机设备实例的shared指针
   * @param[in]  key 指定相机设备名
   * @retval     指定相机设备实例的shared指针
   */
  std::shared_ptr<CameraDevice> GetShared(
      const std::string& key, const CameraImageCallBack& cb);
  /**
   * @brief      用于获取相机设备工厂实例（单例）
   * @retval     相机设备工厂实例
   */
  static CameraDeviceFactory& Instance();

 private:
  CameraDevice* Produce(const std::string& key, const CameraImageCallBack& cb);

  CameraDeviceFactory() {}
  CameraDeviceFactory(const CameraDeviceFactory&) = delete;
  CameraDeviceFactory(CameraDeviceFactory&&)      = delete;

 private:
  std::map<std::string, CONSTRUCT> map_;
};

#define AIROS_CAMERA_REG(T) airos_reg_func_str_##T##_
/**
 * @brief      用于注册指定相机设备
 * @param[in]  T 具体相机设备子类型
 * @param[in]  key 注册的相机设备名字
 */
#define AIROS_CAMERA_REG_FACTORY(T, key) \
  static CameraDeviceFactory::Register_t<T> AIROS_CAMERA_REG(T)(key);

}  // namespace device
}  // namespace base
}  // namespace airos
