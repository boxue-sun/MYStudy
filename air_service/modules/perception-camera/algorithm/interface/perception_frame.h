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
#include <Eigen/Geometry>
#include <Eigen/StdVector>

#include "air_service/modules/perception-camera/algorithm/interface/object_detect_info.h"
#include "air_service/modules/perception-camera/algorithm/interface/object_track_info.h"
#include "air_service/modules/perception-camera/algorithm/interface/object_transform_3d.h"
#include "base/common/point.h"
#include "base/cv_image/image.h"

namespace airos {
namespace perception {
namespace camera {

struct ObjectInfo {
  int type_id                = -1;  // 物体类别, 对应 category_list列表
  algorithm::ObjectType type = algorithm::ObjectType::UNKNOWN;
  float type_id_confidence   = 0;     // @typeID 置信度
  std::vector<float> sub_type_probs;  // @brief probability for each sub-type,
                                      // optional， 所有类型的置信度
  algorithm::Box2f box;               // 2d box
  algorithm::SizeShape size;          // 长宽高
  algorithm::Point2f bottom_uv;       // 1点
  float alpha           = 0;          // 朝向
  float direction_alpha = 0;          // 朝向360
  algorithm::TriStatus is_truncated =
      algorithm::TriStatus::UNKNOWN;  // 是否截断（具体见标注文档）
  algorithm::TriStatus is_occluded =
      algorithm::TriStatus::UNKNOWN;  // 是否遮挡（具体见标注文档）
  int caseflag = 0;  // 属于1个点不可见还是2个点不可见的case

  int track_id = -1;             //
  float theta  = 0;              // 角度
  algorithm::Point3f direction;  // 依赖 theta计算出

  Eigen::Vector3f local_center;
  Eigen::Vector3d center;
  float theta_variance = 0.0f;
  Eigen::Matrix3f center_uncertainty;
  algorithm::CubeBox2f pts8;
};

struct PerceptionFrame {
  // timestamp
  double timestamp = 0.0;
  // camera name
  std::string camera_name = "";
  // camera type, fisheye/monocular
  //   base::CameraTypeBroad camera_type = base::CameraTypeBroad::MONOCULAR;
  // frame sequence id
  int frame_id = 0;
  // image
  std::shared_ptr<airos::base::Image8U> image;
  // segmented objects
  std::vector<ObjectInfo> objects;

  std::vector<algorithm::ObjectDetectInfoPtr> detect_ret;

  std::vector<algorithm::ObjectTrackInfoPtr> track_ret;

  std::vector<algorithm::ObjectTransform3DInfo> transform3d_ret;

  Eigen::Matrix3f camera_k_matrix;
  Eigen::Affine3d camera2world_pose;
  Eigen::Vector4d ground_plane_coffe;

  // void Reset() {}
};  // struct PerceptionFrame

}  // namespace camera
}  // namespace perception
}  // namespace airos
