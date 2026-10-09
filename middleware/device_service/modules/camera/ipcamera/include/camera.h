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

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

namespace airos {
namespace base {
namespace device {

enum class Vendor { HIKVISION, DAHUA };

/**
 * @brief 视频回调函数类型
 */
using stream_callback = void (*)(
    bool is_new_stream, unsigned char *buf, unsigned int buflen,
    unsigned int key);

/**
 * @brief 相机信息
 *
 */
struct cameraInfo {
  Vendor vendor;
  std::string ip;
  uint16_t port;
  std::string username;
  std::string password;
  uint8_t index;
};

/**
 * @brief 视频流参数
 *
 */
struct streamParam {
  stream_callback cb;
  uint32_t user_key;
  uint32_t stream_type;
  int32_t stream_channel;
};

/**
 * @brief reset
 * @note 重置相机
 */
class camera {
 public:
  /**
   * @brief init
   *
   */
  static bool init();

  /**
   * @brief uninit
   *
   */
  static bool uninit();

  /**
   * @brief create camera instance
   *
   */
  static std::shared_ptr<camera> create_camera_instance(
      cameraInfo const &camera_info, streamParam const &stream_param);

  /**
   * @brief Construct the camera object
   *
   */
  camera();

  /**
   * @brief Destroy the camera object
   *
   */
  virtual ~camera();

  /**
   * @brief start
   *
   * @note 登录相机，拉取视频流， 设置视频回调
   * @return true
   * @return false
   */
  virtual bool start() = 0;

  /**
   * @brief stop
   *
   * @note 取消视频回调，取消视频流，登出相机
   * @return true
   * @return false
   */
  virtual bool stop() = 0;

  /**
   * @brief reset
   * @note 重置相机(登出相机，重新登录)
   */
  virtual bool reset() = 0;

 private:
  camera(const camera &) = delete;

  camera &operator=(const camera &) = delete;

  camera(camera &&) = delete;

  camera &operator=(camera &&) = delete;
};

}  // namespace device
}  // namespace base
}  // namespace airos
