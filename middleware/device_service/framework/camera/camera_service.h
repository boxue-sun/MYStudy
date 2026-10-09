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

#include <condition_variable>
#include <map>
#include <memory>
#include <queue>
#include <vector>

#include <Eigen/Geometry>

#include "base/device_connect/camera/camera_base.h"

namespace airos {
namespace middleware {
namespace device_service {

class CameraImpl;

class Camera {
 public:
  using CameraImageData = base::device::CameraImageData;
  friend class CameraService;

  Camera();
  ~Camera();
  std::shared_ptr<CameraImageData> GetCameraData(
      unsigned int timeout_ms = 0);  // 0 means block

 private:
  bool Init(const base::device::CameraInitConfig &config);
  Camera(const Camera &) = delete;
  Camera(Camera &&)      = delete;

 private:
  std::shared_ptr<CameraImpl> impl_;
};

struct CameraIntrinsicParam {
  int camera_width  = 0;
  int camera_height = 0;
  std::vector<double> D;
  std::vector<double> K;
};

struct CameraParam {
  CameraIntrinsicParam intrinsic_params;
  Eigen::Affine3d camera2world_pose;
  Eigen::Vector4d ground_plane_coffe;
};

struct CameraAllInfo {
  CameraParam param;
  std::shared_ptr<airos::base::Image8U> background_img;
  std::shared_ptr<Camera> camera;
  bool inited = false;
};

class CameraService {
 public:
  using CameraImageData = base::device::CameraImageData;

  static CameraService &get_instance();
  bool Init();
  bool InitCamera(const std::string &camera_name, int gpu_id = 0);
  bool QueryAllCameras(std::vector<std::string> &output);
  std::shared_ptr<CameraImageData> GetCameraData(
      const std::string &camera_name,
      unsigned int timeout_ms = 0);  // 0 means block
  bool GetCameraParam(const std::string &camera_name, CameraParam *output);
  bool GetBackgroundImage(
      const std::string &camera_name,
      std::shared_ptr<airos::base::Image8U> &output);  // TODO

 private:
  CameraService()                      = default;
  ~CameraService()                     = default;
  CameraService(const CameraService &) = delete;
  CameraService(CameraService &&)      = delete;

 private:
  std::map<std::string, CameraAllInfo> camera_infos_;
  std::map<std::string, base::device::CameraInitConfig> camera_init_conf_;
  bool init_flag_ = false;
};

}  // namespace device_service
}  // namespace middleware
}  // namespace airos
