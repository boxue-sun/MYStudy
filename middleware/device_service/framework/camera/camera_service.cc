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

#include "camera_service.h"

#include "camera_impl.h"
// #include <Eigen/Geometry>
#include <string.h>

#include <iostream>

#include <yaml-cpp/yaml.h>

#include "base/common/log.h"
#include "base/env/env.h"
#include "base/io/file_util.h"

namespace airos {
namespace middleware {
namespace device_service {

static const char *const DEFAULT_USER     = "admin";
static const char *const DEFAULT_PASSWORD = "";

// bool CameraService::LoadExtrinsics(const std::string &yaml_file,
// Eigen::Matrix4d& extrinsic) {
static bool LoadExtrinsics(
    const std::string &yaml_file, Eigen::Affine3d &camera2world_pose) {
  airos::base::FileUtil file_util;
  if (!file_util.Exists(yaml_file)) {
    LOG_INFO << yaml_file << " not exist!";
    return false;
  }

  YAML::Node node = YAML::LoadFile(yaml_file);
  double qw       = 0.0;
  double qx       = 0.0;
  double qy       = 0.0;
  double qz       = 0.0;
  double tx       = 0.0;
  double ty       = 0.0;
  double tz       = 0.0;
  std::string trans_frame_id, trans_child_frame_id;
  try {
    if (node.IsNull()) {
      LOG_INFO << "Load " << yaml_file << " failed! please check!";
      return false;
    }
    qw                   = node["transform"]["rotation"]["w"].as<double>();
    qx                   = node["transform"]["rotation"]["x"].as<double>();
    qy                   = node["transform"]["rotation"]["y"].as<double>();
    qz                   = node["transform"]["rotation"]["z"].as<double>();
    tx                   = node["transform"]["translation"]["x"].as<double>();
    ty                   = node["transform"]["translation"]["y"].as<double>();
    tz                   = node["transform"]["translation"]["z"].as<double>();
    trans_frame_id       = node["header"]["frame_id"].as<std::string>();
    trans_child_frame_id = node["child_frame_id"].as<std::string>();
  } catch (YAML::InvalidNode &in) {
    LOG_ERROR << "load camera extrisic file " << yaml_file
              << " with error, YAML::InvalidNode exception";
    return false;
  } catch (YAML::TypedBadConversion<double> &bc) {
    LOG_ERROR << "load camera extrisic file " << yaml_file
              << " with error, YAML::TypedBadConversion exception";
    return false;
  } catch (YAML::Exception &e) {
    LOG_ERROR << "load camera extrisic file " << yaml_file
              << " with error, YAML exception:" << e.what();
    return false;
  }
  // extrinsic.setConstant(0);
  Eigen::Quaterniond Q_temp(qw, qx, qy, qz);
  Eigen::Matrix3d rotation_world2camera = Q_temp.matrix().transpose();
  Eigen::Vector3d t_camera2world(tx, ty, tz);
  Eigen::Translation3d translation(
      t_camera2world(0, 0), t_camera2world(1, 0), t_camera2world(2, 0));
  camera2world_pose = translation * rotation_world2camera.transpose();
  // extrinsic.block<3, 3>(0, 0) = Q_temp.normalized().toRotationMatrix();
  // extrinsic(0, 3) = tx;
  // extrinsic(1, 3) = ty;
  // extrinsic(2, 3) = tz;
  // extrinsic(3, 3) = 1;
  //   if (frame_id != nullptr) {
  //     *frame_id = trans_frame_id;
  //   }
  //   if (child_frame_id != nullptr) {
  //     *child_frame_id = trans_child_frame_id;
  //   }
  return true;
}

static bool LoadIntrinsics(
    const std::string &yaml_file, CameraIntrinsicParam &intrinsic_params) {
  airos::base::FileUtil file_util;
  if (!file_util.Exists(yaml_file)) {
    return false;
  }

  YAML::Node node = YAML::LoadFile(yaml_file);
  if (node.IsNull()) {
    LOG_INFO << "Load " << yaml_file << " failed! please check!";
    return false;
  }

  try {
    intrinsic_params.camera_width  = node["width"].as<int>();
    intrinsic_params.camera_height = node["height"].as<int>();
    intrinsic_params.K.resize(node["K"].size());
    for (size_t i = 0; i < node["K"].size(); ++i) {
      intrinsic_params.K[i] = node["K"][i].as<float>();
    }

    intrinsic_params.D.resize(node["D"].size());
    for (size_t i = 0; i < node["D"].size(); ++i) {
      intrinsic_params.D[i] = node["D"][i].as<float>();
    }
  } catch (YAML::Exception &e) {
    LOG_ERROR << "load camera intrisic file " << yaml_file
              << " with error, YAML exception: " << e.what();
    return false;
  }
  return true;
}

static bool LoadGroundPlaneCoffe(
    const std::string &yaml_file, Eigen::Vector4d &ground_plane_coffe) {
  airos::base::FileUtil file_util;
  if (!file_util.Exists(yaml_file)) {
    LOG_ERROR << yaml_file << " not exist! please check!";
    return false;
  }
  YAML::Node node = YAML::LoadFile(yaml_file);
  if (node.IsNull()) {
    LOG_ERROR << "Load " << yaml_file << " failed! please check!";
    return false;
  }
  try {
    ground_plane_coffe(0, 0) = node["a"].as<float>();
    ground_plane_coffe(1, 0) = node["b"].as<float>();
    ground_plane_coffe(2, 0) = node["c"].as<float>();
    ground_plane_coffe(3, 0) = node["d"].as<float>();
  } catch (YAML::Exception &e) {
    LOG_ERROR << "load camera ground file " << yaml_file
              << " with error, YAML exception: " << e.what();
    return false;
  }
  return true;
}

static bool LoadCameraConfig(
    const std::string &yaml_file,
    std::map<std::string, base::device::CameraInitConfig> &camera_confs) {
  airos::base::FileUtil file_util;
  if (!file_util.Exists(yaml_file)) {
    LOG_ERROR << yaml_file << " not exist! please check!";
    return false;
  }
  YAML::Node node = YAML::LoadFile(yaml_file);
  if (node.IsNull()) {
    LOG_ERROR << "Load " << yaml_file << " failed! please check!";
    return false;
  }
  try {
    YAML::Node::const_iterator it = node.begin();
    for (; it != node.end(); ++it) {
      base::device::CameraInitConfig config;
      std::string camera_name = it->first.as<std::string>();
      config.camera_name      = camera_name;

      if (it->second["camera_manufactor"]) {
        config.camera_manufactor =
            it->second["camera_manufactor"].as<std::string>();
      } else {
        config.camera_manufactor =
            "hikvision";  // hikvision dahua simulation_camera
      }
      if (it->second["lens_type"] &&
          it->second["lens_type"].as<std::string>() == "FisheyeLens") {
        config.lens_type = base::device::CameraLensType::FisheyeLens;
      } else {
        config.lens_type = base::device::CameraLensType::PinholeLens;
      }

      if (it->second["img_mode"]) {
        if (it->second["img_mode"].as<std::string>() == "BGR")
          config.img_mode = airos::base::Color::BGR;
        else if (it->second["img_mode"].as<std::string>() == "RGB")
          config.img_mode = airos::base::Color::RGB;
        else if (it->second["img_mode"].as<std::string>() == "GRAY")
          config.img_mode = airos::base::Color::GRAY;
        else
          config.img_mode = airos::base::Color::BGR;
      } else {
        config.img_mode = airos::base::Color::BGR;
      }

      if (it->second["user"]) {
        config.user = it->second["user"].as<std::string>();
      } else {
        config.user = DEFAULT_USER;
      }

      if (it->second["password"]) {
        config.password = it->second["password"].as<std::string>();
      } else {
        config.password = DEFAULT_PASSWORD;
      }

      if (it->second["ip"]) {
        config.ip = it->second["ip"].as<std::string>();
      }

      if (it->second["channel_num"]) {
        config.channel_num = it->second["channel_num"].as<int>();
      } else {
        config.channel_num = 1;
      }

      if (it->second["stream_num"]) {
        config.stream_num = it->second["stream_num"].as<int>();
      } else {
        config.stream_num = 0;
      }

      // if (it->second[D_CAMERA_SN]) {
      //   config.camera_sn = it->second[D_CAMERA_SN].as<std::string>();
      // }
      // if (it->second[D_URL]) {
      //   config.url = it->second[D_URL].as<std::string>();
      // }
      camera_confs[camera_name] = config;
    }
  } catch (YAML::Exception &e) {
    LOG_ERROR << "load camera ground file " << yaml_file
              << " with error, YAML exception: " << e.what();
    return false;
  }
  return true;
}

using CameraImageData = base::device::CameraImageData;

Camera::Camera()
    : impl_(new CameraImpl()) {}

bool Camera::Init(const base::device::CameraInitConfig &config) {
  return impl_->Init(config);
}

std::shared_ptr<CameraImageData> Camera::GetCameraData(
    unsigned int timeout_ms) {
  return impl_->GetCameraData(timeout_ms);
}

Camera::~Camera() = default;

CameraService &CameraService::get_instance() {
  static CameraService instance;
  return instance;
}

bool CameraService::Init() {
  const std::string CMAREA_PARAM_PATH =
      airos::base::Environment::GetPublicParamPath() + "/device/camera/";
  if (init_flag_) return true;
  if (!airos::base::FileUtil::Exists(CMAREA_PARAM_PATH)) return false;

  std::string camera_cfg = CMAREA_PARAM_PATH + "camera.yaml";
  if (LoadCameraConfig(camera_cfg, camera_init_conf_) == false) return false;

  for (auto &camera_cfg : camera_init_conf_) {
    auto &camera_name                 = camera_cfg.first;
    camera_infos_[camera_name].camera = std::make_shared<Camera>();
    std::string extrinsics =
        CMAREA_PARAM_PATH + camera_name + "_extrinsics.yaml";
    std::string intrinsics =
        CMAREA_PARAM_PATH + camera_name + "_intrinsics.yaml";
    std::string coffe_ground =
        CMAREA_PARAM_PATH + camera_name + "_coffe_ground.yaml";
    if (LoadExtrinsics(
            extrinsics, camera_infos_[camera_name].param.camera2world_pose) ==
        false)
      return false;
    if (LoadIntrinsics(
            intrinsics, camera_infos_[camera_name].param.intrinsic_params) ==
        false)
      return false;
    if (LoadGroundPlaneCoffe(
            coffe_ground,
            camera_infos_[camera_name].param.ground_plane_coffe) == false)
      return false;
    // TODO background image
  }
  init_flag_ = true;
  return true;
}

bool CameraService::InitCamera(const std::string &camera_name, int gpu_id) {
  if (!init_flag_) return false;
  if (camera_infos_.find(camera_name) == camera_infos_.end() ||
      camera_init_conf_.find(camera_name) == camera_init_conf_.end())
    return false;
  auto &conf  = camera_init_conf_[camera_name];
  conf.gpu_id = gpu_id;
  return camera_infos_[camera_name].camera->Init(conf);
}

bool CameraService::QueryAllCameras(std::vector<std::string> &output) {
  if (!init_flag_) return false;
  output.clear();
  for (auto &cfg : camera_init_conf_) {
    output.push_back(cfg.first);
  }
  return true;
}

std::shared_ptr<CameraImageData> CameraService::GetCameraData(
    const std::string &camera_name, unsigned int timeout_ms) {
  if (!init_flag_ || camera_infos_.find(camera_name) == camera_infos_.end())
    return nullptr;
  return camera_infos_[camera_name].camera->GetCameraData(timeout_ms);
}

bool CameraService::GetCameraParam(
    const std::string &camera_name, CameraParam *output) {
  if (output == nullptr || !init_flag_ ||
      camera_infos_.find(camera_name) == camera_infos_.end())
    return false;
  *output = camera_infos_[camera_name].param;
  return true;
}

bool CameraService::GetBackgroundImage(
    const std::string &camera_name,
    std::shared_ptr<airos::base::Image8U> &output) {
  if (!init_flag_ || camera_infos_.find(camera_name) == camera_infos_.end())
    return false;
  output = camera_infos_[camera_name].background_img;
  return true;
}

}  // namespace device_service
}  // namespace middleware
}  // namespace airos
