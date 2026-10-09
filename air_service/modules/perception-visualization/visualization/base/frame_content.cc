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

#include "air_service/modules/perception-visualization/visualization/base/frame_content.h"

namespace airos {
namespace perception {
namespace visualization {

int FrameContent::s_max_data_buffer_size_   = 30;
double FrameContent::s_max_buffer_interval_ = 0.4;
int FrameContent::s_frame_idx_              = 0;

int FrameContent::get_s_frame_idx() {
  return s_frame_idx_;
}

void FrameContent::set_s_frame_idx(const int idx) {
  s_frame_idx_ = idx;
}

FrameContent::FrameContent() {
  // fusion_buffer_.set_capacity(s_max_data_buffer_size_);
  global_position_offset_.setZero();
}

void FrameContent::SyncSensorContentsByTimestamp(const double ref_timestamp) {
  SyncContentBuffers<CameraContent>(
      ref_timestamp, &camera_buffers_, &cur_camera_buf_iterators_,
      s_max_buffer_interval_);
}

void FrameContent::AddCameraContent(
    const std::shared_ptr<CameraContent>& content) {
  if (!content) {
    return;
  }
  const std::string sensor_name = content->sensor_name();
  auto& buffer                  = camera_buffers_[sensor_name];
  if (buffer.empty()) {
    buffer.set_capacity(s_max_data_buffer_size_);
  }
  buffer.push_back(*content);
  if (!global_position_offset_inited_) {
    global_position_offset_[0]     = -buffer.back().pose_camera_to_world_(0, 3);
    global_position_offset_[1]     = -buffer.back().pose_camera_to_world_(1, 3);
    global_position_offset_[2]     = -buffer.back().pose_camera_to_world_(2, 3);
    global_position_offset_inited_ = true;
  }
}

// void FrameContent::AddRadarContent(
//     const std::shared_ptr<RadarContent>& content) {
//   if (!content) {
//     return;
//   }
//   const std::string sensor_name = content->sensor_name();
//   auto &buffer = radar_buffers_[sensor_name];
//   if (buffer.empty()) {
//     buffer.set_capacity(s_max_data_buffer_size_);
//   }
//   buffer.push_back(*content);
//   if (!global_position_offset_inited_) {
//     global_position_offset_[0] =
//         -buffer.back().pose_radar_to_world_(0, 3);
//     global_position_offset_[1] =
//         -buffer.back().pose_radar_to_world_(1, 3);
//     global_position_offset_[2] =
//         -buffer.back().pose_radar_to_world_(2, 3);
//     global_position_offset_inited_ = true;
//   }
// }

// void FrameContent::AddUltrasonicContent(
//     const std::shared_ptr<UltrasonicContent>& content) {
//   if (!content) {
//     return;
//   }
//   const std::string sensor_name = content->sensor_name();
//   auto &buffer = ultrasonic_buffers_[sensor_name];
//   if (buffer.empty()) {
//     buffer.set_capacity(s_max_data_buffer_size_);
//   }
//   buffer.push_back(*content);
//   if (!global_position_offset_inited_) {
//     global_position_offset_[0] =
//         -buffer.back().pose_ultrasonic_to_world_(0, 3);
//     global_position_offset_[1] =
//         -buffer.back().pose_ultrasonic_to_world_(1, 3);
//     global_position_offset_[2] =
//         -buffer.back().pose_ultrasonic_to_world_(2, 3);
//     global_position_offset_inited_ = true;
//   }
// }

// void FrameContent::AddFusionContent(
//     const std::shared_ptr<FusionContent>& content) {
//   if (!content) {
//     return;
//   }
//   auto &buffer = fusion_buffer_;
//   if (buffer.empty()) {
//     buffer.set_capacity(s_max_data_buffer_size_);
//   }
//   buffer.push_back(*content);
// }

// void FrameContent::AddGtContent(
//   const std::shared_ptr<FusionContent>& content) {
//   if (!content) {
//     return;
//   }

//   auto &buffer = gt_buffer_;
//   if (buffer.empty()) {
//     buffer.set_capacity(s_max_data_buffer_size_);
//   }
//   buffer.push_back(*content);
// }

// void FrameContent::SetMapContent(
//     const std::shared_ptr<MapContent>& content) {
//   if (!content) {
//     return;
//   }
//   map_content_ = *content;
//   // offset map content
//   for (auto &polygon : map_content_.roi_polygons_) {
//     const int polygon_pt_size = polygon.size();
//     for (int i = 0; i < polygon_pt_size; ++i) {
//       polygon[i].x += global_position_offset_(0);
//       polygon[i].y += global_position_offset_(1);
//       polygon[i].z += global_position_offset_(2);
//     }
//   }
//   for (auto &road_boundary : map_content_.road_boundaries_) {
//     const int left_boundary_pt_size = road_boundary.left_boundary.size();
//     for (int i = 0; i < left_boundary_pt_size; ++i) {
//       auto &pt = road_boundary.left_boundary[i];
//       pt.x += global_position_offset_(0);
//       pt.y += global_position_offset_(1);
//       pt.z += global_position_offset_(2);
//     }
//     const int right_boundary_pt_size = road_boundary.right_boundary.size();
//     for (int i = 0; i < right_boundary_pt_size; ++i) {
//       auto &pt = road_boundary.right_boundary[i];
//       pt.x += global_position_offset_(0);
//       pt.y += global_position_offset_(1);
//       pt.z += global_position_offset_(2);
//     }
//   }
// }

// base::PointFCloudPtr FrameContent::get_lidar_cloud_and_tracked_objects(
//     const std::string& sensor_name,
//     std::vector<camera::ObjectInfo>* lidar_tracked_objects,
//     bool is_original_cloud) const {
//   if (cur_lidar_buf_iterators_.find(sensor_name) ==
//       cur_lidar_buf_iterators_.end() ||
//       lidar_buffers_.find(sensor_name) ==
//       lidar_buffers_.end() ||
//       cur_lidar_buf_iterators_.at(sensor_name) ==
//       lidar_buffers_.at(sensor_name).end()) {
//     return nullptr;
//   }
//   *lidar_tracked_objects =
//     cur_lidar_buf_iterators_.at(sensor_name)->lidar_tracked_objects_;
//   base::PointFCloudPtr cloud = is_original_cloud ?
//     (cur_lidar_buf_iterators_.at(sensor_name)->point_cloud_ptr_):
//     (cur_lidar_buf_iterators_.at(sensor_name)->roi_point_cloud_ptr_);
//   return cloud;
// }

// Eigen::Matrix4d FrameContent::get_lidar_to_world_pose(
//     const std::string& sensor_name) const {
//   if (cur_lidar_buf_iterators_.find(sensor_name) ==
//         cur_lidar_buf_iterators_.end() ||
//       lidar_buffers_.find(sensor_name) ==
//         lidar_buffers_.end() ||
//       cur_lidar_buf_iterators_.at(sensor_name) ==
//         lidar_buffers_.at(sensor_name).end()) {
//     return Eigen::Matrix4d::Identity();
//   }
//   return cur_lidar_buf_iterators_.at(sensor_name)->pose_lidar_to_world_;
// }

// double FrameContent::get_lidar_timestamp(
//     const std::string& sensor_name) const {
//   if (cur_lidar_buf_iterators_.find(sensor_name) ==
//         cur_lidar_buf_iterators_.end() ||
//       lidar_buffers_.find(sensor_name) ==
//         lidar_buffers_.end() ||
//       cur_lidar_buf_iterators_.at(sensor_name) ==
//         lidar_buffers_.at(sensor_name).end()) {
//     return 0.0;
//   }
//   return cur_lidar_buf_iterators_.at(sensor_name)->timestamp();
// }

const std::vector<camera::ObjectInfo>& FrameContent::get_camera_objects(
    const std::string& sensor_name) const {
  if (cur_camera_buf_iterators_.find(sensor_name) ==
          cur_camera_buf_iterators_.end() ||
      camera_buffers_.find(sensor_name) == camera_buffers_.end() ||
      cur_camera_buf_iterators_.at(sensor_name) ==
          camera_buffers_.at(sensor_name).end()) {
    return empty_objects_;
  }
  return cur_camera_buf_iterators_.at(sensor_name)->camera_objects_;
}

// const std::vector<base::TrafficLightPtr>&
// FrameContent::get_camera_trafficlights(
//     const std::string& sensor_name) const {
//   if (cur_camera_buf_iterators_.find(sensor_name) ==
//       cur_camera_buf_iterators_.end() ||
//     camera_buffers_.find(sensor_name) ==
//       camera_buffers_.end() ||
//     cur_camera_buf_iterators_.at(sensor_name) ==
//       camera_buffers_.at(sensor_name).end()) {
//     return empty_trafficlights_;
//   }
//   return cur_camera_buf_iterators_.at(sensor_name)->trafficlights_;
// }

// const std::vector<base::TrafficLightPtr>&
// FrameContent::get_camera_projectiontrafficlights(
//     const std::string& sensor_name) const {
//   if (cur_camera_buf_iterators_.find(sensor_name) ==
//       cur_camera_buf_iterators_.end() ||
//     camera_buffers_.find(sensor_name) ==
//       camera_buffers_.end() ||
//     cur_camera_buf_iterators_.at(sensor_name) ==
//       camera_buffers_.at(sensor_name).end()) {
//     return empty_trafficlights_;
//   }
//   return
//   cur_camera_buf_iterators_.at(sensor_name)->projection_trafficlights_;
// }

// const std::vector<base::CalibPointsGroundInImagePtr>&
// FrameContent::get_camera_calib_points(
//     const std::string& sensor_name) const {
//   if (cur_camera_buf_iterators_.find(sensor_name) ==
//       cur_camera_buf_iterators_.end() ||
//     camera_buffers_.find(sensor_name) ==
//       camera_buffers_.end() ||
//     cur_camera_buf_iterators_.at(sensor_name) ==
//       camera_buffers_.at(sensor_name).end()) {
//     return empty_calibpoints_ground_;
//   }
//   return cur_camera_buf_iterators_.at(sensor_name)->calib_points_ground_;
// }

// const std::vector<base::LaneLine>& FrameContent::get_camera_lane_objects(
//       const std::string& sensor_name) const {
//   if (cur_camera_buf_iterators_.find(sensor_name) ==
//         cur_camera_buf_iterators_.end() ||
//       camera_buffers_.find(sensor_name) ==
//         camera_buffers_.end() ||
//       cur_camera_buf_iterators_.at(sensor_name) ==
//         camera_buffers_.at(sensor_name).end()) {
//     return empty_lane_objects_;
//   }
//   return cur_camera_buf_iterators_.at(sensor_name)->lane_objects_;
// }

std::shared_ptr<airos::base::Blob<uint8_t>>
FrameContent::get_camera_color_image(const std::string& sensor_name) const {
  if (cur_camera_buf_iterators_.find(sensor_name) ==
          cur_camera_buf_iterators_.end() ||
      camera_buffers_.find(sensor_name) == camera_buffers_.end() ||
      cur_camera_buf_iterators_.at(sensor_name) ==
          camera_buffers_.at(sensor_name).end()) {
    return nullptr;
  }
  return cur_camera_buf_iterators_.at(sensor_name)->image_data_ptr_;
}

// TODO(guiyilin): get range image

Eigen::Matrix4d FrameContent::get_camera_to_world_pose(
    const std::string& sensor_name) const {
  if (cur_camera_buf_iterators_.find(sensor_name) ==
          cur_camera_buf_iterators_.end() ||
      camera_buffers_.find(sensor_name) == camera_buffers_.end() ||
      cur_camera_buf_iterators_.at(sensor_name) ==
          camera_buffers_.at(sensor_name).end()) {
    return Eigen::Matrix4d::Identity();
  }
  return cur_camera_buf_iterators_.at(sensor_name)->pose_camera_to_world_;
}

Eigen::Matrix3f FrameContent::get_camera_intrinsic_params(
    const std::string& sensor_name) const {
  if (cur_camera_buf_iterators_.find(sensor_name) ==
          cur_camera_buf_iterators_.end() ||
      camera_buffers_.find(sensor_name) == camera_buffers_.end() ||
      cur_camera_buf_iterators_.at(sensor_name) ==
          camera_buffers_.at(sensor_name).end()) {
    return Eigen::Matrix3f::Identity();
  }
  return cur_camera_buf_iterators_.at(sensor_name)->intrinsic_params_;
}

// base::CameraFrameSupplementConstPtr
// FrameContent::get_camera_frame_supplement(
//     const std::string& sensor_name) const {
//   if (cur_camera_buf_iterators_.find(sensor_name) ==
//         cur_camera_buf_iterators_.end() ||
//       camera_buffers_.find(sensor_name) ==
//         camera_buffers_.end() ||
//       cur_camera_buf_iterators_.at(sensor_name) ==
//         camera_buffers_.at(sensor_name).end()) {
//     return nullptr;
//   }
//   return cur_camera_buf_iterators_.at(sensor_name)->
//       camera_frame_supplement_ptr_;
// }

double FrameContent::get_camera_timestamp(
    const std::string& sensor_name) const {
  if (cur_camera_buf_iterators_.find(sensor_name) ==
          cur_camera_buf_iterators_.end() ||
      camera_buffers_.find(sensor_name) == camera_buffers_.end() ||
      cur_camera_buf_iterators_.at(sensor_name) ==
          camera_buffers_.at(sensor_name).end()) {
    return 0.0;
  }
  return cur_camera_buf_iterators_.at(sensor_name)->timestamp();
}

// const std::vector<camera::ObjectInfo>& FrameContent::get_radar_objects(
//     const std::string& sensor_name) const {
//   if (cur_radar_buf_iterators_.find(sensor_name) ==
//         cur_radar_buf_iterators_.end() ||
//       radar_buffers_.find(sensor_name) ==
//         radar_buffers_.end() ||
//       cur_radar_buf_iterators_.at(sensor_name) ==
//         radar_buffers_.at(sensor_name).end()) {
//     return empty_objects_;
//   }
//   return cur_radar_buf_iterators_.at(sensor_name)->radar_objects_;
// }

// base::RadarPointCloudPtr FrameContent::get_radar_raw_frame(
//   const std::string& sensor_name) const {
//   if (cur_radar_buf_iterators_.find(sensor_name) ==
//         cur_radar_buf_iterators_.end() ||
//       radar_buffers_.find(sensor_name) ==
//         radar_buffers_.end() ||
//       cur_radar_buf_iterators_.at(sensor_name) ==
//         radar_buffers_.at(sensor_name).end()) {
//     return nullptr;
//   }
//   return cur_radar_buf_iterators_.at(
//     sensor_name)->radar_frame_supplement.radar_raw_data_ptr;
// }

// Eigen::Matrix4d FrameContent::get_radar_to_world_pose(
//     const std::string& sensor_name) const {
//   if (cur_radar_buf_iterators_.find(sensor_name) ==
//         cur_radar_buf_iterators_.end() ||
//       radar_buffers_.find(sensor_name) ==
//         radar_buffers_.end() ||
//       cur_radar_buf_iterators_.at(sensor_name) ==
//         radar_buffers_.at(sensor_name).end()) {
//     return Eigen::Matrix4d::Identity();
//   }
//   return cur_radar_buf_iterators_.at(sensor_name)->pose_radar_to_world_;
// }

// double FrameContent::get_radar_timestamp(
//     const std::string& sensor_name) const {
//   if (cur_radar_buf_iterators_.find(sensor_name) ==
//         cur_radar_buf_iterators_.end() ||
//       radar_buffers_.find(sensor_name) ==
//         radar_buffers_.end() ||
//       cur_radar_buf_iterators_.at(sensor_name) ==
//         radar_buffers_.at(sensor_name).end()) {
//     return 0.0;
//   }
//   return cur_radar_buf_iterators_.at(sensor_name)->timestamp();
// }

// const std::vector<base::ImpendingCollisionEdgePtr>&
// FrameContent::get_ultrasonic_objects(const std::string& sensor_name) const {
//   if (cur_ultrasonic_buf_iterators_.find(sensor_name) ==
//         cur_ultrasonic_buf_iterators_.end() ||
//       ultrasonic_buffers_.find(sensor_name) ==
//         ultrasonic_buffers_.end() ||
//       cur_ultrasonic_buf_iterators_.at(sensor_name) ==
//         ultrasonic_buffers_.at(sensor_name).end()) {
//     return empty_ultrasonic_objects_;
//   }
//   return cur_ultrasonic_buf_iterators_.at(sensor_name)->objects_;
// }

// const std::vector<base::ImpendingCollisionEdgePtr>&
// FrameContent::get_ultrasonic_original_objects(
//     const std::string& sensor_name) const {
//   if (cur_ultrasonic_buf_iterators_.find(sensor_name) ==
//         cur_ultrasonic_buf_iterators_.end() ||
//       ultrasonic_buffers_.find(sensor_name) ==
//         ultrasonic_buffers_.end() ||
//       cur_ultrasonic_buf_iterators_.at(sensor_name) ==
//         ultrasonic_buffers_.at(sensor_name).end()) {
//     return empty_ultrasonic_objects_;
//   }
//   return cur_ultrasonic_buf_iterators_.at(sensor_name)->original_objects_;
// }

// Eigen::Matrix4d FrameContent::get_ultrasonic_to_world_pose(
//     const std::string& sensor_name) const {
//   if (cur_ultrasonic_buf_iterators_.find(sensor_name) ==
//         cur_ultrasonic_buf_iterators_.end() ||
//       ultrasonic_buffers_.find(sensor_name) ==
//         ultrasonic_buffers_.end() ||
//       cur_ultrasonic_buf_iterators_.at(sensor_name) ==
//         ultrasonic_buffers_.at(sensor_name).end()) {
//     return Eigen::Matrix4d::Identity();
//   }
//   return cur_ultrasonic_buf_iterators_.at(sensor_name)->
//       pose_ultrasonic_to_world_;
// }

// double FrameContent::get_ultrasonic_timestamp(
//     const std::string& sensor_name) const {
//   if (cur_ultrasonic_buf_iterators_.find(sensor_name) ==
//         cur_ultrasonic_buf_iterators_.end() ||
//       ultrasonic_buffers_.find(sensor_name) ==
//         ultrasonic_buffers_.end() ||
//       cur_ultrasonic_buf_iterators_.at(sensor_name) ==
//         ultrasonic_buffers_.at(sensor_name).end()) {
//     return 0.0;
//   }
//   return cur_ultrasonic_buf_iterators_.at(sensor_name)->timestamp();
// }

// const std::vector<camera::ObjectInfo>&
//     FrameContent::get_fused_objects() const {
//   if (current_fusion_itr_ == fusion_buffer_.end()) {
//     return empty_objects_;
//   }
//   return current_fusion_itr_->fused_objects_;
// }

// const std::vector<camera::ObjectInfo>&
//     FrameContent::get_gt_objects() const {
//   if (current_gt_iter_ == gt_buffer_.end()) {
//     return empty_objects_;
//   }
//   return current_gt_iter_->fused_objects_;
// }

// double FrameContent::get_fusion_timestamp() const {
//   if (current_fusion_itr_ == fusion_buffer_.end()) {
//     return 0.0;
//   }
//   return current_fusion_itr_->timestamp();
// }

// double FrameContent::get_gt_timestamp() const {
//   if (current_gt_iter_ == gt_buffer_.end()) {
//     return 0.0;
//   }
//   return current_gt_iter_->timestamp();
// }

}  // namespace visualization
}  // namespace perception
}  // namespace airos
