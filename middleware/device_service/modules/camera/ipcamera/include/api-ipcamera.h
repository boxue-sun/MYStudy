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

#pragma once

#ifndef __cplusplus
#error "This is a c++ header!"
#endif

#include <cstdint>
#include <memory>
#include <string>

#include "base/device_connect/camera/camera_base.h"
#include "ipcamera/include/api-structs.h"
/// 离线播包模式，开启则不会连接相机，转而监听 channel 并读取 H.264 数据。
#define API_MODE_OFFLINE (0x10000000U)
/// 开启则不会转发 FFMPEG 的 avlog 到 glog 中。
#define API_MODE_DISABLE_REDIRECT_AVLOG (0x04000000U)

namespace airos {
namespace base {
namespace device {

/**
 * @brief                      获取一条 RGB 数据，带有1秒同步阻塞功能
 * @param[in] channel_str      相机 channel，初始化时设置
 * @retval                     一条 RGB 数据
 */
std::shared_ptr<GPUImage> api_get_image(const std::string &channel_str)
    __attribute__((__used__));

/**
 * @brief 初始化一个 Driver 实例
 * @param[in] drv_mode 初始化模式，默认值为0。赋值时请使用上述宏定义。例：
 *                          API_MODE_OFFLINE | API_MODE_DISABLE_REDIRECT_AVLOG
 * @param[in] max_timeout_us    最大允许延迟 微秒
 * @param[in] maxnb_ready_delay ready 最大等待的包数
 * @param[in] ipport            相机的地址，例："103.125.47.36"
 * @param[in] channel_str       相机 channel ，当作 KEY 值使用
 * @param[in] cudadev           GPU 编号，默认使用 0 号
 * @param[in] imgmode           图片颜色模式
 * @param[in] vendor            相机厂家,当前支持 "hikvision"、"dahua"
 * @param[in] stream_num        码流号
 * @param[in] channel_num       通道号
 * @param[in] frame_cache_cnt
 * 帧缓存数量，取值范围[1，3]，缓存数量越多端到端延时越大，默认为3兼容老版本
 * @param[in] username          相机登陆用户名
 * @param[in] passwd            相机登陆密码
 * @retval                  是否成功初始化
 */
__attribute__((__used__)) bool api_init_instance(
    uint32_t drv_mode, const std::string &ipport,
    const std::string &channel_str, int32_t cudadev = 0,
    airos::base::Color imgmode = airos::base::Color::RGB,
    const std::string &vendor = "hikvision", int stream_num = 0,
    int channel_num = 1, const std::string &username = "",
    const std::string &passwd = "", const CameraImageCallBack cb = nullptr);

}  // END namespace device
}  // END namespace base
}  // namespace airos
