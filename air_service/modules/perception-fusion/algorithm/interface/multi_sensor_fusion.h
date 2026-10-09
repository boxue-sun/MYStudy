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

#include <memory>
#include <set>
#include <unordered_map>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/StdVector>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include "air_service/modules/perception-fusion/algorithm/air_fusion/base/common.h"

namespace bg = boost::geometry;
typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
typedef bg::model::polygon<point_t, false, false> polygon_t;
namespace airos {
namespace perception {
namespace msf {

enum class InputType {
  CAMERA    = 0,
  LIDAR     = 1,
  RADAR     = 2,
  OTHERS    = 3,
  OLD       = 4,
  VEHICLE   = 5,
  FISHEYE   = 6,
  CARSTATUS = 7
};

struct Point2f {
  float x = 0;
  float y = 0;
};
struct Point3f {
  float x = 0;
  float y = 0;
  float z = 0;
};

struct Box2f {
  float Width() const {
    return right_bottom.x - left_top.x;
  }
  float Height() const {
    return right_bottom.y - left_top.y;
  }
  Point2f left_top;
  Point2f right_bottom;
};

struct SizeShape {
  float length = 0;
  float width  = 0;
  float height = 0;
};

struct PolygonInfo {
  Eigen::Vector3d polygon;
  Eigen::Matrix3d variance;
};

struct PerceptionObject {
  base::SensorType sensor_type = base::SensorType::UNKNOWN_SENSOR_TYPE;
  double timestamp             = 0.0;  // sensor timestamp
  std::string sensor_name;
  int track_id = -1;

  // @brief object type, required
  base::ObjectType type = base::ObjectType::UNKNOWN;
  // @brief probability for each type, required
  std::vector<float> type_probs;
  // @brief object sub-type, optional
  base::ObjectSubType sub_type = base::ObjectSubType::UNKNOWN;
  // @brief probability for each sub-type, optional
  std::vector<float> sub_type_probs;
  // @brief  2d box
  Box2f box;

  base::OcclusionState occ_state = base::OcclusionState::OCC_UNKNOWN;  // 2
  // for  Info3d position;
  Eigen::Vector3d center;
  Eigen::Matrix3d center_uncertainty;
  // for radar
  double main_theta = -10;
  // for Infod theta;
  float theta          = 0.0f;
  float theta_variance = 0.5f;
  SizeShape size;  // 长宽高
  std::vector<PolygonInfo> polygon;
  // for Info3d velocity;  //
  Point3f velocity;  // 速度，虚拟速度
  Eigen::Matrix3d velocity_uncertainty;
  bool is_truncation  = false;  // 2
  bool collision_line = false;
};

struct ObjectMeasurement {
  std::string sensor_name = "unknown_sensor";
  double timestamp        = 0.0;
  int track_id            = -1;
  Box2f box;
};

struct FusionInput {
  InputType input_type;
  int index                  = -1;
  bool use_header_time       = false;
  bool main_sensor           = true;
  bool collision_line_fusion = false;
  double timestamp           = -1.0;
  int sequence_num           = -1;
  int consumed_times         = -1;
  std::string sensor_name;
  std::vector<PerceptionObject> objects;
};

struct FusionObject {
  double timestamp = 0.0;  // fusion timestamp

  double tracking_time = 0.0;

  int track_id = -1;  // fusion track_id

  std::vector<ObjectMeasurement> measurements;

  // @brief object type, required
  base::ObjectType type = base::ObjectType::UNKNOWN;

  // @brief probability for each type, required
  std::vector<float> type_probs;
  // @brief object sub-type, optional
  base::ObjectSubType sub_type = base::ObjectSubType::UNKNOWN;
  // @brief probability for each sub-type, optional
  std::vector<float> sub_type_probs;

  std::string lane_id = "None";

  // for  Info3d position;
  Eigen::Vector3d center;
  Eigen::Matrix3d center_uncertainty;

  // for Infod theta;
  float theta          = 0.0f;
  float theta_variance = 0.5f;

  SizeShape size;  // 长宽高

  Point3f velocity;  // 速度，虚拟速度

  bool collision_line = false;

  std::vector<Eigen::Vector3d> polygon;
};

struct FusionOutput {
  std::vector<FusionObject> objects;
};

class BaseMultiSensorFusion {
 public:
  struct InitParam {
    unsigned int channel_num;
    double time_diff              = -1.0;
    double missing_time_threshold = -1.0;
    double reverse_time_threshold = -1.0;
    bool collision_line_check     = false;
    std::set<int> reference_seq;
    std::shared_ptr<std::unordered_map<std::string, polygon_t>> d3_rois;
    std::string fusion_params_file;
  };

 public:
  virtual bool Init(const InitParam&) = 0;
  virtual int Process(
      const std::vector<FusionInput>& input, FusionOutput& output) = 0;

  virtual double Timestamp() const = 0;
};

}  // namespace msf
}  // namespace perception
}  // namespace airos
