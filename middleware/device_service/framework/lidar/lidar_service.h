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

#include <Eigen/Geometry>

#include "base/common/msg_queue.h"
#include "base/device_connect/lidar/device_base.h"

namespace airos {
namespace middleware {
namespace device_service {

using PointCloud = os::v2x::device::PointCloud;
using PointCloudQueue =
    std::shared_ptr<base::common::MsgQueue<std::shared_ptr<const PointCloud>>>;

struct LidarCalibParam {
  Eigen::Affine3d lidar2world_pose;
};

class LidarService {
 public:
  static LidarService &get_instance();
  bool InitLidar(const std::string &lidar_name);
  std::shared_ptr<const PointCloud> GetLidarData(
      unsigned int timeout_ms = 0);  // 0 means block
  bool GetLidarParam(LidarCalibParam *output);

 private:
  LidarService()                     = default;
  ~LidarService()                    = default;
  LidarService(const LidarService &) = delete;
  LidarService(LidarService &&)      = delete;

  bool InitDriver(const std::string &lidar_name);
  bool LoadLidarConfig(
      const std::string &yaml_file, const std::string &lidar_name);
  bool LoadExtrinsics(
      const std::string &yaml_file, Eigen::Affine3d &lidar2world_pose);
  void ReceiveCallBack(const std::shared_ptr<const PointCloud> &data);

 private:
  os::v2x::device::LidarInitConfig lidar_conf_;
  LidarCalibParam lidar_calib_param_;
  PointCloudQueue point_cloud_queue_;
  std::shared_ptr<os::v2x::device::LidarDevice> device_;
  bool init_flag_ = false;
};

}  // namespace device_service
}  // namespace middleware
}  // namespace airos
