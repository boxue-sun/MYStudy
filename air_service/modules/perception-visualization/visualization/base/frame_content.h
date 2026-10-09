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
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <Eigen/Core>
#include <boost/circular_buffer.hpp>

#include "air_service/modules/perception-camera/algorithm/interface/perception_frame.h"
#include "base/blob/blob.h"

namespace airos {
namespace perception {
namespace visualization {

class BaseSensorContent {
 public:
  BaseSensorContent()
      : timestamp_(0.0) {}
  virtual ~BaseSensorContent() = default;

  double timestamp() const {
    return timestamp_;
  }

  void set_timestamp(const double ts) {
    timestamp_ = ts;
  }

  std::string sensor_name() const {
    return sensor_name_;
  }

  void set_sensor_name(const std::string& name) {
    sensor_name_ = name;
  }

 private:
  double timestamp_;
  std::string sensor_name_;
};  // class BaseSensorContent

class CameraContent : public BaseSensorContent {
 public:
  CameraContent()
      : pose_camera_to_world_(Eigen::Matrix4d::Identity())
      , intrinsic_params_(Eigen::Matrix3f::Identity())
      , image_data_ptr_(nullptr) {}
  ~CameraContent() = default;

  Eigen::Matrix4d pose_camera_to_world_;
  Eigen::Matrix3f intrinsic_params_;
  std::shared_ptr<airos::base::Blob<uint8_t>> image_data_ptr_;
  std::vector<camera::ObjectInfo> camera_objects_;
};

class FrameContent {
 public:
  FrameContent();
  ~FrameContent() = default;

  FrameContent(const FrameContent&)            = delete;
  FrameContent& operator=(const FrameContent&) = delete;

  void SyncSensorContentsByTimestamp(const double ref_timestamp);

  void AddCameraContent(const std::shared_ptr<CameraContent>& content);

  // Camera content getters
  const std::vector<camera::ObjectInfo>& get_camera_objects(
      const std::string& sensor_name) const;

  std::shared_ptr<airos::base::Blob<uint8_t>> get_camera_color_image(
      const std::string& sensor_name) const;

  Eigen::Matrix4d get_camera_to_world_pose(
      const std::string& sensor_name) const;

  Eigen::Matrix3f get_camera_intrinsic_params(
      const std::string& sensor_name) const;

  double get_camera_timestamp(const std::string& sensor_name) const;

  const Eigen::Vector3d& global_position_offset() const {
    return global_position_offset_;
  }

 private:
  template <typename T>
  void FindClosestInBuffer(
      double timestamp, boost::circular_buffer<T>* buffer,
      typename boost::circular_buffer<T>::iterator* closest_itr,
      double max_interval);

  template <typename T>
  void SyncContentBuffers(
      double timestamp,
      std::unordered_map<std::string, boost::circular_buffer<T>>* buffers,
      std::unordered_map<
          std::string, typename boost::circular_buffer<T>::iterator>*
          buffer_iterators,
      double max_interval);

 public:
  static int s_max_data_buffer_size_;
  static double s_max_buffer_interval_;  // max interval when find buffer

  // TODO(guiyilin): for offline visualization
  static int s_frame_idx_;
  static int get_s_frame_idx();
  static void set_s_frame_idx(const int idx);

 private:
  // Camera
  std::unordered_map<std::string, boost::circular_buffer<CameraContent>>
      camera_buffers_;
  std::unordered_map<
      std::string, boost::circular_buffer<CameraContent>::iterator>
      cur_camera_buf_iterators_;

  Eigen::Vector3d global_position_offset_;
  bool global_position_offset_inited_ = false;

  const std::vector<camera::ObjectInfo> empty_objects_;
};  // class FrameContent

template <typename T>
void FrameContent::FindClosestInBuffer(
    double timestamp, boost::circular_buffer<T>* buffer,
    typename boost::circular_buffer<T>::iterator* closest_itr,
    double max_interval) {
  if ((*buffer).empty()) {
    *closest_itr = (*buffer).end();
    return;
  }
  double min_ts_diff = FLT_MAX;
  double ts_diff     = FLT_MAX;
  int buffer_size    = (*buffer).size();
  *closest_itr       = (*buffer).end();
  for (int i = buffer_size - 1; i >= 0; --i) {
    ts_diff = timestamp + 1.0e-7 - (*buffer)[i].timestamp();
    if (ts_diff >= 0 && ts_diff < min_ts_diff &&
        ts_diff < s_max_buffer_interval_) {
      *closest_itr = (*buffer).begin() + i;
      min_ts_diff  = ts_diff;
    }
  }
}

template <typename T>
void FrameContent::SyncContentBuffers(
    double timestamp,
    std::unordered_map<std::string, boost::circular_buffer<T>>* buffers,
    std::unordered_map<
        std::string, typename boost::circular_buffer<T>::iterator>*
        buffer_iterators,
    double max_interval) {
  for (const auto& kv : (*buffers)) {
    const auto& sensor_name          = kv.first;
    (*buffer_iterators)[sensor_name] = (*buffers).at(sensor_name).end();
    FindClosestInBuffer<T>(
        timestamp, &(buffers->at(sensor_name)),
        &((*buffer_iterators)[sensor_name]), max_interval);
  }
}

}  // namespace visualization
}  // namespace perception
}  // namespace airos
