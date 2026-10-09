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

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>

#include "middleware/device_service/modules/camera/ipcamera/include/camera.h"

namespace airos {
namespace base {
namespace device {

class dahua_camera : public camera {
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
   * @brief construct with param
   *
   */
  explicit dahua_camera(
      cameraInfo const &camera_info, streamParam const &stream_param);

  /**
   * @brief Destroy the camera object
   *
   */
  virtual ~dahua_camera();

  /**
   * @brief start
   *
   * @note 登录相机，拉取视频流， 设置视频回调
   * @return true
   * @return false
   */
  bool start() final;

  /**
   * @brief stop
   *
   * @note 取消视频回调，取消视频流，登出相机
   * @return true
   * @return false
   */
  bool stop() final;

  /**
   * @brief reset
   * @note 重置相机(登出相机，重新登录)
   */
  bool reset() final;

  void stream_callback(unsigned char *buf, unsigned int buflen);

  std::string get_ip();

 private:
  /**
   * @brief login 登录相机
   * @note 参数在构造函数中传入
   */
  bool login();

  /**
   * @brief logout 登出相机
   *
   * @return true
   * @return false
   */
  bool logout();

  /**
   * @brief 开始拉取视频流
   *
   * @return int32_t
   */
  bool start_stream();

  /**
   * @brief 停止拉取视频流
   *
   * @return int32_t
   */
  bool stop_stream();

  dahua_camera() = delete;

  dahua_camera(const dahua_camera &) = delete;

  const dahua_camera &operator=(const dahua_camera &) = delete;

  dahua_camera(dahua_camera &&) = delete;

  dahua_camera &operator=(dahua_camera &&) = delete;

  cameraInfo _camera_info;
  streamParam _stream_param;
  bool _logined;
  bool _fetched;
  std::mutex _mutex;
  std::string _sn;
  int64_t _userId;
  int64_t _streamId;
  bool _is_new_stream;
};

}  // namespace device
}  // namespace base
}  // namespace airos
