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
 * @brief    radar设备接口
 * @version  V1.0.0
 */

#pragma once

#include <functional>
#include <memory>

#include "base/device_connect/proto/radar_data.pb.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @struct  RadarDeviceState
 * @brief   radar设备运行状态
 * @details
 */
enum class RadarDeviceState { UNKNOWN, RUNNING, STOP };

using RadarDataType = std::shared_ptr<const RadarObstacles>;
using RadarCallBack = std::function<void(const RadarDataType&)>;

/**
 * @brief  radar设备接口类
 */
class RadarDevice {
 public:
  RadarDevice() = default;
  /**
   * @brief      radar设备构造函数
   * @param[in]  cb AirOS-edge框架注册的回调函数
   */
  explicit RadarDevice(const RadarCallBack& cb)
      : sender_(cb) {}
  virtual ~RadarDevice() = default;
  /**
   * @brief      用于radar设备初始化
   * @param[in]  config_file radar设备初始化参数配置文件
   * @retval     初始化是否成功
   */
  virtual bool Init(const std::string& config_file) = 0;
  /**
   * @brief      用于启动radar设备，产出AirOS-edge结构化的标准radar输出数据
   */
  virtual void Start() = 0;
  /**
   * @brief      用于获取radar设备状态
   * @retval     设备状态码radarDeviceState
   */
  virtual RadarDeviceState GetState() = 0;

 protected:
  RadarCallBack sender_;
};

}  // namespace device
}  // namespace v2x
}  // namespace os
