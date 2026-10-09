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
 * @brief    lidar设备接口
 * @version  V1.0.0
 */

#pragma once

#include <functional>
#include <memory>

#include "base/device_connect/proto/lidar_data.pb.h"

namespace os {
namespace v2x {
namespace device {

struct LidarInitConfig {
  std::string lidar_name;        ///< Lidar名称
  std::string lidar_manufactor;  ///< Lidar厂商
  std::string model;             ///< Lidar类型
  std::string ip;                ///< ip 地址
};

/**
 * @struct  LidarDeviceState
 * @brief   lidar设备运行状态
 * @details
 */
enum class LidarDeviceState { UNKNOWN, NORMAL, OFFLINE };

using LidarPBDataTypePtr = std::shared_ptr<const PointCloud>;
using LidarCallBack      = std::function<void(const LidarPBDataTypePtr&)>;

/**
 * @brief  lidar设备接口类
 */
class LidarDevice {
 public:
  LidarDevice() = default;
  /**
   * @brief      lidar设备构造函数
   * @param[in]  cb AirOS-edge框架注册的回调函数
   */
  explicit LidarDevice(const LidarCallBack& cb)
      : sender_(cb) {}
  virtual ~LidarDevice() = default;
  /**
   * @brief      用于lidar设备初始化
   * @param[in]  config lidar设备初始化参数
   * @retval     初始化是否成功
   */
  virtual bool Init(const LidarInitConfig& config) = 0;
  /**
   * @brief      用于获取lidar设备状态
   * @retval     设备状态码LidarDeviceState
   */
  virtual LidarDeviceState GetState() = 0;

 protected:
  LidarCallBack sender_;
};

}  // namespace device
}  // namespace v2x
}  // namespace os
