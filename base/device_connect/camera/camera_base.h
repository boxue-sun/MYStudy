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
 * @file     camera_base.h
 * @brief    相机设备接口
 * @version  V1.0.0
 */

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "base/cv_image/image.h"

namespace airos {
namespace base {
namespace device {

// enum class CameraDeviceType { IPCAMERA, CAMERA };
enum class CameraLensType { PinholeLens, FisheyeLens };

/**
 * @struct  CameraInitConfig
 * @brief   driver 初始化参数
 * @details
 */
struct CameraInitConfig {
  std::string camera_name;        ///< 相机名称
  std::string camera_manufactor;  ///< 相机厂商
  CameraLensType lens_type;       ///< 镜头类型
  int gpu_id;                     ///< 解码设备id
  airos::base::Color img_mode;    ///< 图像模式
  std::string user;               ///< 相机登录时用户名
  std::string password;           ///< 相机登录时密码
  std::string ip;                 ///< ip 地址[:端口号]
  int channel_num;                ///< 相机通道索引
  int stream_num;                 ///< 相机流索引
  // int sequence_num;                                         ///<序列号
  // std::string url;                                          ///<拉流地址
};

/**
 * @struct  CameraDeviceState
 * @brief   相机设备状态
 * @details
 */
enum class CameraDeviceState { UNKNOWN, RUNNING, STOP, NO_REGISTER };

/**
 * @struct  CameraData
 * @brief   AIROS结构化的标准相机输出数据
 * @details
 */
struct CameraImageData {
  std::shared_ptr<airos::base::Image8U> image;  ///< 图片指针
  uint32_t device_id;                           ///< 设备ID
  std::string camera_name;                      ///< 相机名称
  std::string camera_manufactor;                ///< 相机厂商
  CameraLensType lens_type;                     ///< 镜头类型
  airos::base::Color mode;                      ///< 图像模式
  uint32_t height;                              ///< 高度
  uint32_t width;                               ///< 宽度
  uint64_t timestamp;                           ///< 时间戳
  uint32_t sequence_num;                        ///< 序列号
};

// using CameraStreamCallBack = std::function<void(const std::string&
// camera_name, const uint8_t* data, size_t size)>;

using CameraImageCallBack =
    std::function<void(const std::shared_ptr<CameraImageData>&)>;

/**
 * @brief  相机接口类
 */
class CameraDevice {
 public:
  explicit CameraDevice(const CameraImageCallBack& cb)
      : image_sender_(cb) {}
  virtual ~CameraDevice() = default;
  /**
   * @brief      用于相机初始化
   * @param[in]  config_file 相机初始化参数配置文件
   * @retval     初始化是否成功
   */
  virtual bool Init(const CameraInitConfig& config) = 0;

 protected:
  CameraImageCallBack image_sender_;
};

}  // namespace device
}  // namespace base
}  // namespace airos
