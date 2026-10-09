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

#include "middleware/device_service/framework/lidar/lidar_service.h"

#include <yaml-cpp/yaml.h>

#include "base/common/log.h"
#include "base/device_connect/lidar/device_factory.h"
#include "base/env/env.h"
#include "base/io/file_util.h"
#include "base/plugin/modules_loader/dynamic_loader.h"

namespace airos {
namespace middleware {
namespace device_service {

LidarService &LidarService::get_instance() {
  static LidarService instance;
  return instance;
}

bool LidarService::InitLidar(const std::string &Lidar_name) {
  const std::string LIDAR_PARAM_PATH =
      airos::base::Environment::GetPublicParamPath() + "/device/lidar/";
  if (init_flag_) return true;
  if (!airos::base::FileUtil::Exists(LIDAR_PARAM_PATH)) return false;
  std::string lidar_cfg = LIDAR_PARAM_PATH + "lidar.yaml";
  if (LoadLidarConfig(lidar_cfg, Lidar_name) == false) return false;

  std::string extrinsics = LIDAR_PARAM_PATH + Lidar_name + "_extrinsics.yaml";
  if (LoadExtrinsics(extrinsics, lidar_calib_param_.lidar2world_pose) == false)
    return false;
  return InitDriver(Lidar_name);
}

bool LidarService::InitDriver(const std::string &Lidar_name) {
  static const int cache_num = 1;
  point_cloud_queue_         = std::make_shared<
              base::common::MsgQueue<std::shared_ptr<const PointCloud>>>(cache_num);
  if (point_cloud_queue_ == nullptr) return false;
  airos::base::DynamicLoader::GetInstance().LoadDevicePackage(
      "device/lib/lidar/");
  device_ = os::v2x::device::LidarDeviceFactory::Instance().GetShared(
      lidar_conf_.lidar_manufactor,
      std::bind(&LidarService::ReceiveCallBack, this, std::placeholders::_1));
  if (device_ == nullptr) {
    return false;
  }
  if (!device_->Init(lidar_conf_)) {
    return false;
  }
  point_cloud_queue_->enable();
  init_flag_ = true;
  return true;
}

bool LidarService::LoadLidarConfig(
    const std::string &yaml_file, const std::string &Lidar_name) {
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
      std::string driver_name = it->first.as<std::string>();
      if (driver_name == Lidar_name) {
        lidar_conf_.lidar_name = driver_name;
        if (it->second["lidar_manufactor"]) {
          lidar_conf_.lidar_manufactor =
              it->second["lidar_manufactor"].as<std::string>();
        } else {
          lidar_conf_.lidar_manufactor = "inno";
        }
        if (it->second["model"]) {
          lidar_conf_.model = it->second["model"].as<std::string>();
        } else {
          lidar_conf_.model = it->second["model"].as<std::string>();
        }
      }
    }
  } catch (YAML::Exception &e) {
    LOG_ERROR << "load lidar file " << yaml_file
              << " with error, YAML exception: " << e.what();
    return false;
  }
  return true;
}

bool LidarService::LoadExtrinsics(
    const std::string &yaml_file, Eigen::Affine3d &lidar2world_pose) {
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
  try {
    if (node.IsNull()) {
      LOG_INFO << "Load " << yaml_file << " failed! please check!";
      return false;
    }
    qw = node["transform"]["rotation"]["w"].as<double>();
    qx = node["transform"]["rotation"]["x"].as<double>();
    qy = node["transform"]["rotation"]["y"].as<double>();
    qz = node["transform"]["rotation"]["z"].as<double>();
    tx = node["transform"]["translation"]["x"].as<double>();
    ty = node["transform"]["translation"]["y"].as<double>();
    tz = node["transform"]["translation"]["z"].as<double>();
  } catch (YAML::InvalidNode &in) {
    LOG_ERROR << "load lidar extrisic file " << yaml_file
              << " with error, YAML::InvalidNode exception";
    return false;
  } catch (YAML::TypedBadConversion<double> &bc) {
    LOG_ERROR << "load lidar extrisic file " << yaml_file
              << " with error, YAML::TypedBadConversion exception";
    return false;
  } catch (YAML::Exception &e) {
    LOG_ERROR << "load lidar extrisic file " << yaml_file
              << " with error, YAML exception:" << e.what();
    return false;
  }
  Eigen::Quaterniond Q_temp(qw, qx, qy, qz);
  Eigen::Matrix3d rotation_world2lidar = Q_temp.matrix().transpose();
  Eigen::Vector3d t_lidar2world(tx, ty, tz);
  Eigen::Translation3d translation(
      t_lidar2world(0, 0), t_lidar2world(1, 0), t_lidar2world(2, 0));
  lidar2world_pose = translation * rotation_world2lidar.transpose();
  return true;
}

std::shared_ptr<const PointCloud> LidarService::GetLidarData(
    unsigned int timeout_ms) {
  std::shared_ptr<const PointCloud> res;
  if (point_cloud_queue_ == nullptr ||
      point_cloud_queue_->pop(
          &res, base::common::PopPolicy::blocking, timeout_ms) != 0) {
    LOG_ERROR << "queue empty";
    return nullptr;
  }
  return res;
}

void LidarService::ReceiveCallBack(
    const std::shared_ptr<const PointCloud> &data) {
  if (point_cloud_queue_)
    point_cloud_queue_->push(data, base::common::PushPolicy::discard_old);
  return;
}

bool LidarService::GetLidarParam(LidarCalibParam *output) {
  if (output == nullptr || !init_flag_) return false;
  *output = lidar_calib_param_;
  return true;
}

}  // namespace device_service
}  // namespace middleware
}  // namespace airos
