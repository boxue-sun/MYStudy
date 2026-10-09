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
 * @brief    红绿灯设备接口
 * @version  V1.0.0
 */

#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "base/device_connect/proto/traffic_light_data.pb.h"
#include "base/device_connect/traffic_light/traffic_flow_data.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @struct  TrafficLightDeviceState
 * @brief   红绿灯采集设备运行状态
 * @details
 */
enum class TrafficLightDeviceState { UNKNOWN, RUNNING, STOP };

using TrafficLightDataType = std::shared_ptr<const TrafficLightBaseData>;
using TrafficLightCallBack = std::function<void(const TrafficLightDataType&)>;

/**
 * @brief  红绿灯采集设备接口类
 */
class TrafficLightDevice {
 public:
  TrafficLightDevice() = default;
  /**
   * @brief      红绿灯采集设备构造函数
   * @param[in]  cb AirOS-edge框架注册的回调函数
   */
  explicit TrafficLightDevice(const TrafficLightCallBack& cb)
      : sender_(cb) {}
  virtual ~TrafficLightDevice() = default;
  /**
   * @brief      用于红绿灯采集设备初始化
   * @param[in]  config_file 红绿灯采集设备初始化参数配置文件
   * @retval     初始化是否成功
   */
  virtual bool Init(const std::string& config_file) = 0;
  /**
   * @brief 用于启动红绿灯采集设备，产出AirOS-edge结构化的标准红绿灯采集输出数据
   */
  virtual void Start() = 0;
  /**
   * @brief      用于获取红绿灯采集设备状态
   * @retval     设备状态码红绿灯采集DeviceState
   */
  virtual TrafficLightDeviceState GetState() = 0;
  /**
   * @brief      用于将数据写入红绿灯采集设备状态
   * @param[in]  re_proto AirOS-edge结构化的标准红绿灯采集写入数据
   */
  virtual void WriteToDevice(
      const std::shared_ptr<const TrafficLightReceiveData>& re_proto) = 0;

  /**
   * @brief      将脉冲数据发送给信号机
   * @param[in]  lanes 通道列表，每项 pair<通道序号, 车辆方向(0离开/1进入)>
   */
  virtual void WritePulseData(
      const std::vector<std::pair<uint8_t, uint8_t>>& lanes) {}

  /**
   * @brief      将5分钟统计流量数据（A13）发送给信号机
   * @param[in]  coils 线圈统计数据列表
   */
  virtual void WriteTrafficFlowData(
      const std::vector<CoilFlowData>& coils) {}

 protected:
  TrafficLightCallBack sender_;
};

}  // namespace device
}  // namespace v2x
}  // namespace os
