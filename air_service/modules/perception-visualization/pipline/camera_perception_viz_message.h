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
#include <string>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "air_service/framework/proto/airos_perception_obstacle.pb.h"

#include "air_service/modules/perception-camera/algorithm/interface/perception_frame.h"
#include "base/blob/blob.h"

namespace airos {
namespace perception {
namespace visualization {

class CameraPerceptionVizMessage {
 public:
  CameraPerceptionVizMessage()  = default;
  ~CameraPerceptionVizMessage() = default;

  std::string GetTypeName() const {
    return "CameraPerceptionVizMessage";
  }

  CameraPerceptionVizMessage(
      const std::string& camera_name, const double msg_timestamp,
      const Eigen::Matrix4d& pose_camera_to_world,
      const Eigen::Matrix3f intrinsic_params,
      const std::shared_ptr<airos::base::Blob<uint8_t>>& image_blob,
      const std::shared_ptr<PerceptionObstacles>& perception_obstacles)
      : camera_name_(camera_name)
      , msg_timestamp_(msg_timestamp)
      , pose_camera_to_world_(pose_camera_to_world)
      , intrinsic_params_(intrinsic_params)
      , image_blob_(image_blob) {
    camera_objects_.clear();
    for (int i = 0; i < perception_obstacles->perception_obstacle_size(); i++) {
      const PerceptionObstacle& obstacle =
          perception_obstacles->perception_obstacle(i);
      camera::ObjectInfo camera_obj;
      camera_obj.track_id           = obstacle.id();
      camera_obj.box.left_top.x     = obstacle.bbox2d().xmin();
      camera_obj.box.left_top.y     = obstacle.bbox2d().ymin();
      camera_obj.box.right_bottom.x = obstacle.bbox2d().xmax();
      camera_obj.box.right_bottom.y = obstacle.bbox2d().ymax();
      camera_obj.size.length        = obstacle.length();
      camera_obj.size.width         = obstacle.width();
      camera_obj.size.height        = obstacle.height();
      camera_obj.theta              = obstacle.theta();
      camera_obj.center             = {
          obstacle.position().x(), obstacle.position().y(),
          obstacle.position().z()};
      camera_obj.is_occluded = algorithm::TriStatus::UNKNOWN;

      camera_obj.direction.x = obstacle.cube_pts8(0).x();
      camera_obj.direction.y = obstacle.cube_pts8(0).y();
      camera_obj.direction.z = obstacle.cube_pts8(0).z();
      camera_obj.center[0]   = obstacle.cube_pts8(1).x();
      camera_obj.center[1]   = obstacle.cube_pts8(1).y();
      camera_obj.center[2]   = obstacle.cube_pts8(1).z();

      if (obstacle.has_occ_state()) {
        switch (obstacle.occ_state()) {
          case OcclusionState::OCC_UNKNOWN:
            camera_obj.is_occluded = algorithm::TriStatus::UNKNOWN;
            break;
          case OcclusionState::OCC_COMPLETE_OCCLUDED:
            camera_obj.is_occluded = algorithm::TriStatus::TRUE;
            break;
          case OcclusionState::OCC_NONE_OCCLUDED:
            camera_obj.is_occluded = algorithm::TriStatus::FALSE;
            break;
          default:
            break;
        }
      }
      camera_obj.is_truncated = algorithm::TriStatus::UNKNOWN;
      if (obstacle.has_trunc_state()) {
        switch (obstacle.trunc_state()) {
          case TruncationState::TRUNC_UNKNOWN:
            camera_obj.is_truncated = algorithm::TriStatus::UNKNOWN;
            break;
          case TruncationState::TRUNC_TRUE:
            camera_obj.is_truncated = algorithm::TriStatus::TRUE;
            break;
          case TruncationState::TRUNC_FALSE:
            camera_obj.is_truncated = algorithm::TriStatus::FALSE;
            break;
          default:
            break;
        }
      }
      camera_obj.theta_variance = obstacle.theta_variance();
      for (int j = 0; j < obstacle.cube_pts8_size(); j++) {
        camera_obj.pts8.pts8[j].x = obstacle.cube_pts8(j).x();
        camera_obj.pts8.pts8[j].y = obstacle.cube_pts8(j).y();
      }
      camera_obj.type = static_cast<algorithm::ObjectType>(obstacle.sub_type());
      for (int index = 0; index < obstacle.sub_type_probs_size(); index++) {
        camera_obj.sub_type_probs.push_back(
            obstacle.sub_type_probs(index).prob());
      }
      camera_obj.type_id_confidence = obstacle.sub_type_id_confidence();
      camera_obj.type_id            = obstacle.sub_type_id();
      camera_objects_.push_back(camera_obj);
    }
  }

 public:
  std::string camera_name_;
  double msg_timestamp_ = 0.0;
  Eigen::Matrix4d pose_camera_to_world_;
  Eigen::Matrix3f intrinsic_params_;
  // std::shared_ptr<base::Image8U> image;
  std::shared_ptr<airos::base::Blob<uint8_t>> image_blob_;

  std::vector<camera::ObjectInfo> camera_objects_;  // TODO
};

}  // namespace visualization
}  // namespace perception
}  // namespace airos
