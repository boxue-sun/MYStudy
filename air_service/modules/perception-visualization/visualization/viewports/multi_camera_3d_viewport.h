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

#ifndef IDG_PERCEPTION_VISUALIZATION_GL_FUSION_VISUALIZER_VIEWPORTS_MULTI_CAMERA_3D_VIEWPORT_H_  // NOLINT
#define IDG_PERCEPTION_VISUALIZATION_GL_FUSION_VISUALIZER_VIEWPORTS_MULTI_CAMERA_3D_VIEWPORT_H_  // NOLINT

#include <string>
#include <vector>

#include "air_service/modules/perception-visualization/visualization/base/frame_content.h"
#include "air_service/modules/perception-visualization/visualization/viewports/base_gl_viewport.h"
#include "air_service/modules/perception-visualization/visualization/viewports/viewport_utils.h"

namespace airos {
namespace perception {
namespace visualization {

class MultiCamera3DViewport : public BaseGLViewport {
 public:
  MultiCamera3DViewport() {}
  ~MultiCamera3DViewport() {}

 protected:
  void draw() override;

 private:
  // void DrawCloud();
  void DrawCircles();
  void DrawCarForwardDir();
  void DrawFrontCameraFOV(
      const std::string& camera_name, double fov_region_radius,
      const Eigen::Vector3d& color, double z_offset);
  void DrawHudText();  // draw 2d heads up display text
                       // void DrawUltrasonic();

  // void DrawCameraLaneResults(
  //     const std::vector<base::LaneLine>& lane_results,
  //     const Eigen::Matrix4d& c2w_pose);

 private:
  // VAO and VBO for point cloud
  static const int kVAOCloudNum_ = 35;
  static const int kVBOCloudNum_ = 10000;
  GLuint vao_cloud_[kVAOCloudNum_];
  GLuint buffers_cloud_[kVAOCloudNum_][visualization::VBO_TYPE::NUM_VBOS];
  GLfloat cloud_verts_[kVBOCloudNum_][3];

  // VAO and VBO for circles
  static const int kVAOCircleNum_ = 10;
  static const int kVBOCircleNum_ = 360;
  GLuint vao_circle_[kVAOCircleNum_];

  bool inited_ = false;
};

}  // namespace visualization
}  // namespace perception
}  // namespace airos

#endif  // IDG_PERCEPTION_VISUALIZATION_GL_FUSION_VISUALIZER_VIEWPORTS_MULTI_CAMERA_3D_VIEWPORT_H_
        // // NOLINT
