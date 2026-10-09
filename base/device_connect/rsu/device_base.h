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
 * @brief    rsu设备接口
 * @version  V1.0.0
 */

#pragma once

#include <functional>
#include <memory>

#include "base/device_connect/proto/rsu_data.pb.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @struct  RSUDeviceState
 * @brief   rsu设备运行状态
 * @details
 */
enum class RSUDeviceState { UNKNOWN, NORMAL, OFFLINE };

using RSUPBDataType = std::shared_ptr<const RSUData>;
using RSUCallBack   = std::function<void(const RSUPBDataType&)>;

/**
 * @brief  rsu设备接口类
 */
class RSUDevice {
 public:
  RSUDevice() = default;
  /**
   * @brief      rsu设备构造函数
   * @param[in]  cb AirOS-edge框架注册的回调函数
   */
  explicit RSUDevice(const RSUCallBack& cb)
      : sender_(cb) {}
  virtual ~RSUDevice() = default;
  /**
   * @brief      用于rsu设备初始化
   * @param[in]  config_file rsu设备初始化参数配置文件
   * @retval     初始化是否成功
   */
  virtual bool Init(const std::string& conf_file) = 0;
  /**
   * @brief      用于启动rsu设备，产出AirOS-edge结构化的标准rsu输出数据
   */
  virtual void Start() = 0;
  /**
   * @brief      用于获取rsu设备状态
   * @retval     设备状态码rsuDeviceState
   */
  virtual RSUDeviceState GetState() = 0;
  /**
   * @brief      用于将数据写入rsu设备状态
   * @param[in]  re_proto AirOS-edge结构化的标准rsu写入数据
   */
  virtual void WriteToDevice(
      const std::shared_ptr<const RSUData>& re_proto) = 0;

 protected:
  RSUCallBack sender_;
};

}  // end of namespace device
}  // end of namespace v2x
}  // end of namespace os
