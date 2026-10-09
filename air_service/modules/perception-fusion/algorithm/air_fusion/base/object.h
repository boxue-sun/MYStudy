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

#include <iomanip>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <Eigen/Core>
#include <boost/circular_buffer.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include "absl/strings/str_cat.h"
#include "air_service/modules/perception-fusion/algorithm/air_fusion/base/object_supplement.h"
#include "air_service/modules/perception-fusion/algorithm/air_fusion/base/object_types.h"

namespace airos {
namespace perception {
namespace msf {

namespace bg = boost::geometry;
typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
typedef bg::model::polygon<point_t, false, false> polygon_t;

namespace base {

template <typename Val, typename Var>
class Info {
 public:
  Info() = default;
  Info(Val val, Var var) {
    value_    = val;
    variance_ = var;
  }
  ~Info() = default;
  // Info(const Info &) = delete;
  Info(const Info& rhs) {
    value_    = rhs.value_;
    variance_ = rhs.variance_;
  };
  // Info &operator=(const Info &) = delete;
  Info& operator=(const Info& rhs) {
    value_    = rhs.value_;
    variance_ = rhs.variance_;

    return *this;
  };

  Val Value() const {
    return value_;
  }

  Var Variance() const {
    return variance_;
  }

  void Set(Val value, Var variance) {
    value_    = value;
    variance_ = variance;
  }

  void SetVariance(Var variance) {
    variance_ = variance;
  }

 protected:
  Val value_;
  Var variance_;
};

class Info3d : public Info<Eigen::Vector3d, Eigen::Matrix3d> {
 public:
  Info3d() {
    value_.setZero();
    variance_.setZero();
  }
  ~Info3d() = default;
  double x() const {
    return value_(0);
  }

  double y() const {
    return value_(1);
  }

  double z() const {
    return value_(2);
  }

  double length() const {
    return value_(0);
  }

  double width() const {
    return value_(1);
  }

  double height() const {
    return value_(2);
  }
};

struct Trajectory {
  double x          = 0.0;
  double y          = 0.0;
  double z          = 0.0;
  double time_stamp = 0.0;
  double theta      = 0.0;
  double velocity   = 0.0;
  double velocity_x = 0.0;
  double velocity_y = 0.0;
  bool is_predicted = false;
  int is_abnormal   = 0;
};

struct LaneFeature {
  std::string lane_id           = "";
  int lane_turn_type            = 0;
  double lane_s                 = 0.0;
  double lane_l                 = 0.0;
  double angle_diff             = 0.0;
  double dist_to_left_boundary  = 0.0;
  double dist_to_right_boundary = 0.0;
  double lane_heading           = 0.0;
  int lane_type                 = 0;
};

typedef Info<bool, float> Infob;
typedef Info<float, float> Infof;
typedef Info<double, double> Infod;
typedef Info<Eigen::Vector2f, Eigen::Matrix2f> Info2f;
typedef Info<Eigen::Vector2d, Eigen::Matrix2d> Info2d;
typedef Eigen::Vector2f Point2f;
typedef Eigen::Vector3f Point3f;

struct alignas(16) Object {
  Object()
      : is_position_credible(true, 0.5)
      ,  // 182
      theta(0.0, -1.0)
      ,  // 188
      map_theta(0.0, -1.0)
      ,  // 190
      is_velocity_converged(false, -1.0)
      ,  // 224
      is_stationary(false, -1.0)
      ,  // 226
      is_temporary_lost(false, -1.0)
      ,  // 228
      is_predicted(false, -1.0)
      ,                               // 230
      is_size_credible(true, 0.8) {}  // 232
  ~Object()                        = default;
  Object(const Object&)            = default;
  Object& operator=(const Object&) = default;
  bool operator<(const Object& rhs) const {
    return timestamp < rhs.timestamp;
  }
  bool operator>(const Object& rhs) const {
    return timestamp > rhs.timestamp;
  }
  bool operator==(const Object& rhs) const {
    return timestamp == rhs.timestamp;
  }

  std::string ToString() const;
  void Reset();
  // camera, lidar, radar and others
  SensorType sensor_type = SensorType::UNKNOWN_SENSOR_TYPE;  // 2
  // ROADSIDE
  // @brief sensor-specific object supplements, optional
  CameraObjectSupplement camera_supplement;  // 2
  FusionObjectSupplement fusion_supplement;  // 3

  // message timestamp
  double timestamp = 0.0;  // 4

  // @brief age of the tracked object, required
  double tracking_time = 0.0;  // 3

  std::string frame_id = "";  // 4, input: equal camera_supplement->sensor_name
  // @brief track id, required
  int track_id = -1;  // 4 as input, equal camera_supplement local_track_id,
                      // output is fusion_track_id
                      // @breif object id per frame, required
                      // int global_id = -1;   //1
  bool collision_line = false;  // 4

  std::string lane_id = "None";        // 3
  std::set<std::string> successor_id;  // 5
  std::set<std::string> neighbor_id;   // 5
  // @brief center position of the boundingbox (x, y, z), required
  Info3d position;  // 4
  // @brief 观测是否可信
  Infob is_position_credible;  // 5
  // @brief anchor point, required
  // Info3d anchor_point;  //1

  double main_theta = -10;  // 2
  // -Pi~Pi from east
  Infod theta;  // 4
  // -Pi~Pi from east
  Infod map_theta;  // 5
  // @brief main direction of the object, required
  // Info3d direction;  //1
  /*@brief the yaw angle, theta = 0.0 <=> direction(1, 0, 0),
    currently roll and pitch are not considered,
    make sure direction and theta are consistent, required
  */

  /* @brief size = [length, width, height] of boundingbox
     length is the size of the main direction, required
  */
  Info3d size;  // 4
  // @brief convex hull of the object, required
  // Point3f polygon;

  // @brief object type, required
  ObjectType type = ObjectType::UNKNOWN;  // 4

  // @brief probability for each type, required
  std::vector<float> type_probs;  // 4
  // @brief object sub-type, optional
  ObjectSubType sub_type = ObjectSubType::UNKNOWN;  // 4
  // @brief probability for each sub-type, optional
  std::vector<float> sub_type_probs;  // 4

  std::vector<std::vector<Info3d>> tentacles;  // 5

  std::vector<Info3d> polygon;  // 4

  std::vector<Trajectory> trajectory;  // 1
  // tracking information
  // @brief velocity of the object, required
  Info3d velocity;  // 4
  // @brief 匀速直线运动的物体的速度是否收敛、可信
  Infob is_velocity_converged;  // 4

  Infob is_stationary;  // 5

  Infob is_temporary_lost;  // 5

  Infob is_predicted;  // 5

  Infob is_size_credible;  // 5

  OcclusionState occ_state = OcclusionState::OCC_UNKNOWN;  // 2
  // fusion information

  // @brief std::pair<std::string camera_id, int object id>
  std::vector<std::tuple<std::string, int, double>> fused_objects;  // 5

  std::vector<Trajectory> predicted_trajectory;  // 1
  bool is_on_lane = false;                       // 5
  // bool is_near_junction = false;  //1
  bool is_truncation = false;  // 2
  bool is_repeat_obj = false;  // 5

  std::string reserved;  // 1
  int index = -1;        // 5

  std::string DebugString() const {
    std::ostringstream time_string, x_string, y_string;
    time_string << std::fixed;
    time_string << std::setprecision(3);
    x_string << std::fixed;
    x_string << std::setprecision(3);
    y_string << std::fixed;
    y_string << std::setprecision(3);
    time_string << timestamp;
    x_string << position.x();
    y_string << position.y();

    std::string debug_string = absl::StrCat(
        "frame id: ", frame_id,
        ", ",  //
        "time: ", time_string.str(),
        ", ",  //
        "id: ", track_id,
        ", ",  //
        "sensor type: ", sensor_type,
        ", ",  //
        "type: ", kObjectType2NameMap.at(type),
        ", ",  //
        "subtype: ", kSubType2NameMap.at(sub_type),
        ", ",  //
        "x: ", x_string.str(),
        ", ",  //
        "y: ", y_string.str(),
        ", ",  //
        "z: ", position.z(),
        ", ",  //
        "vx: ", velocity.x(),
        ", ",  //
        "vy: ", velocity.y(),
        ", ",  //
        "vz: ", velocity.z(),
        ", ",  //
        "theta: ", theta.Value(),
        ", ",  //
        "length: ", size.length(),
        ", ",  //
        "width: ", size.width(),
        ", ",  //
        "height: ", size.height(),
        ", ",  //
        "is predicted: ", is_predicted.Value() ? "true" : "false",
        ", "  //
        "predict time: ",
        is_predicted.Variance(),
        ", ",  //
        "reserved: ", reserved);
    std::stringstream ss;
    ss << position.Variance();
    debug_string =
        absl::StrCat(debug_string, ", ", "covariance: \n", ss.str(), "\n");
    return debug_string;
  }
};

}  // namespace base
typedef std::shared_ptr<base::Object> ObjectPtr;
typedef std::shared_ptr<const base::Object> ObjectConstPtr;
}  // namespace msf
}  // namespace perception
}  // namespace airos
