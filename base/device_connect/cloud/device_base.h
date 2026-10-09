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
 * @file     device_base.h
 * @brief    cloud设备接口
 * @version  V1.0.0
 */

#pragma once

#include <functional>
#include <memory>

#include "base/device_connect/proto/cloud_data.pb.h"

#include "yaml-cpp/yaml.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @struct  CloudDeviceState
 * @brief   Cloud设备运行状态
 * @details
 */
enum class CloudDeviceState { UNKNOWN, NORMAL, OFFLINE };

using CloudPBDataType = std::shared_ptr<const CloudData>;
using CloudCallBack   = std::function<void(const CloudPBDataType&)>;

/**
 * @brief  Cloud设备接口类
 */
class CloudDevice {
 public:
  CloudDevice() = default;
  /**
   * @brief      Cloud设备构造函数
   * @param[in]  cb AirOS-edge框架注册的回调函数
   */
  explicit CloudDevice(const CloudCallBack& cb)
      : sender_(cb) {}
  virtual ~CloudDevice() = default;
  /**
   * @brief      用于Cloud设备初始化
   * @param[in]  config_file Cloud设备初始化参数配置文件
   * @retval     初始化是否成功
   */
  virtual bool Init(const YAML::Node& root_node) = 0;
  /**
   * @brief      用于启动Cloud设备，产出AirOS-edge结构化的标准Cloud输出数据
   */
  virtual void Start() = 0;
  /**
   * @brief      用于获取Cloud设备状态
   * @retval     设备状态码CloudDeviceState
   */
  virtual CloudDeviceState GetState() = 0;
  /**
   * @brief      用于将数据写入Cloud设备状态
   * @param[in]  re_proto AirOS-edge结构化的标准Cloud写入数据
   */
  virtual void WriteToDevice(
      const std::shared_ptr<const CloudData>& re_proto) = 0;

 protected:
  CloudCallBack sender_;
};

}  // end of namespace device
}  // end of namespace v2x
}  // end of namespace os
