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
#include <map>
#include <mutex>
#include <string>

#include "arpa/inet.h"
#include "middleware/device_service/modules/camera/ipcamera/include/camera.h"

namespace airos {
namespace base {
namespace device {

class hikvision_camera : public camera {
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
  explicit hikvision_camera(
      cameraInfo const &camera_info, streamParam const &stream_param);

  /**
   * @brief Destroy the camera object
   *
   */
  virtual ~hikvision_camera();

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

  void *get_analyzedata();

  double get_framerate();

  void set_analyzedata(void *);

  void set_framerate(double);

  std::string get_ip();

  void callback(bool is_new_stream, unsigned char *buf, unsigned int buflen);

  /**
   * @brief 根据key获取相机
   *
   * @return true
   * @return false
   */
  static hikvision_camera *get_camera_by_key(int key);

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

  /**
   * @brief 设置视频流回调
   *
   * @return true
   * @return false
   */
  bool set_stream_cb();

  /**
   * @brief 取消视频流回调
   *
   * @return true
   * @return false
   */
  bool unset_stream_cb();

  hikvision_camera() = delete;

  hikvision_camera(const hikvision_camera &) = delete;

  const hikvision_camera &operator=(const hikvision_camera &) = delete;

  hikvision_camera(hikvision_camera &&) = delete;

  hikvision_camera &operator=(hikvision_camera &&) = delete;

  static std::map<int, hikvision_camera *> _camera_set;  // 相机集合<ip,camera>
  static std::mutex _mtx;

  cameraInfo _camera_info;
  streamParam _stream_param;
  void *_analyze_data;
  double _frame_rate;

  bool _logined;
  bool _fetched;
  std::mutex _mutex;
  std::string _sn;
  int _userId;
  int _streamId;
};

}  // namespace device
}  // namespace base
}  // namespace airos
