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
#include <sys/stat.h>
#include <sys/time.h>

#include <memory>
#include <mutex>
#include <unordered_map>

#include <Eigen/Eigenvalues>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>

#include "air_service/modules/perception-fusion/algorithm/interface/multi_sensor_fusion.h"

namespace airos {
namespace perception {
namespace msf {
class WindowWrapper {
 public:
  WindowWrapper() = default;
  ~WindowWrapper();
  void Init(int mode, double chisquare = 10.0);

  void CalculateScale(const std::vector<msf::PerceptionObject>& objects);
  void CalculateScale(const std::vector<msf::FusionObject>& objects);
  void DrawInputObstacles(std::vector<msf::FusionInput>& fusion_input_list);
  void DrawOutputObstacles(msf::FusionOutput& fusion_output);
  void DrawObstaclesError(double x, double y, double error);
  void Show();
  void KeyCallbackFunc(int key);
  void MouseCallBackFunc();
  static void ModeChangeCallBackFunc(int number, void* usedata);
  void SetManual() {
    manual_ = true;
  }
  void ResetBackground() {
    image_ = cv::Mat(height_, width_, CV_8UC3, cv::Scalar::all(255));
  }
  const int Mode() const {
    return mode_;
  }
  const bool ShowRealTimeResult() const {
    return show_real_time_switch_;
  }
  const bool ShowOtherResult() const {
    return old_result_switch_;
  }

 public:
  int mode_ = 0;

 private:
  void Resize();
  void Drag(int key);
  void Drag(int x, int y);
  void DragUp();
  void DragDown();
  void DragLeft();
  void DragRight();
  void Zoom(int key);
  void ZoomIn();
  void ZoomOut();
  void RestBorder();

  cv::Scalar ChooseColor(const msf::base::ObjectType& type, int sensor_id);
  cv::Point2d GetCenter();
  cv::Point2d GetTopLeft();
  void DrawPolygon(const cv::Scalar& color);
  void DrawBoldPolygon(const cv::Scalar& color);
  void DrawDashPolygon(const cv::Scalar& color);

  bool IsInPolygon(const std::vector<cv::Point2i>& polygon, cv::Point2d point);

  void DrawYaw(
      const cv::Point2d& point1, float length, float yaw,
      const cv::Scalar& color);

  void DrawSpeed(
      const msf::Point3f& velocity, const cv::Point2d& point,
      const cv::Scalar& color);

  void DrawOccState(
      const msf::PerceptionObject& object, const cv::Point2d& point,
      const cv::Scalar& color);
  void DrawTruncation(
      const msf::PerceptionObject& object, const cv::Point2d& point,
      const cv::Scalar& color);

  void DrawId(
      const int& track_id, const cv::Point2d& point, const cv::Scalar& color);
  void DrawPolygon(
      const std::vector<cv::Point2i>& vertices, const cv::Scalar& color,
      int thickness, cv::LineTypes line_type);
  void DrawError(
      const Eigen::Matrix3d& center_var, const cv::Point2d& point,
      const cv::Scalar& color);
  void DrawCameraId(
      const cv::Point2d& point, int sensor_id, const cv::Scalar& color);
  void DrawType(
      const base::ObjectType& type, const cv::Point2d& point,
      const cv::Scalar& color);
  void DrawSubType(
      const base::ObjectSubType& sub_type, const cv::Point2d& point,
      const cv::Scalar& color);
  void DrawRange(const cv::Point2d& point, const cv::Scalar& color);
  void GetObjectPolygon(
      const msf::SizeShape& shape, float theta, Eigen::Vector3d center);

  cv::Point2d TranslatePosition(const cv::Point2d& point);
  cv::Point2d InverseTranslatePosition(const cv::Point2d& point);
  void RecordLogPolygon(bool reset = false);
  void DrawLogPolygon();
  void AddRuler(cv::Point2i point, bool head);

  void CVUI();

 private:
  int camera_switch_                    = 0;
  bool show_real_time_switch_           = true;
  bool old_result_switch_               = false;
  bool id_switch_                       = false;
  bool lane_id_switch_                  = false;
  bool trajectory_switch_               = false;
  bool predicted_trajectory_switch_     = false;
  bool tentacles_switch_                = false;
  bool camera_id_switch_                = false;
  bool type_switch_                     = false;
  bool attribute_switch_                = false;
  bool sub_type_switch_                 = false;
  bool error_circle_switch_             = false;
  bool error_circle_eigen_value_switch_ = false;
  bool shadow_region_switch_            = false;
  int chisquare_level_                  = 200;
  double chisquare_                     = 10.0;
  bool speed_switch_                    = false;
  bool predict_switch_                  = false;
  bool predictor_future_switch_         = false;
  bool occ_state_switch_                = false;
  bool type_probs_show_switch_          = false;
  bool log_swtich_                      = false;
  bool coordinates_switch_              = false;
  // int dashboard_switch_ = 0;
#ifdef MSF_DEBUG
  bool show_car_perception_switch_ = false;
  bool car_status_switch_          = false;
#endif
  bool reserved_switch_ = false;
  bool plate_switch_    = false;

 private:
  const double to_degree_ = 180.0 / M_PI;
  const float kscale_id_  = 25;
  std::vector<cv::Point2i> vertices_;
  // image coordinates
  const int plugin_area_ = 150;
  int height_            = 1080;
  int width_             = 1080 + plugin_area_;
  // world coordinates
  // double center_x_ = 0.0;
  // double center_y_ = 0.0;
  std::mutex border_mutex_;
  double border_x_min_ = 1e9;
  double border_y_min_ = 1e9;
  double border_x_max_ = 0;
  double border_y_max_ = 0;
  // tansition
  double scale_          = 1;
  bool manual_           = false;
  bool inputting_id_     = false;
  bool need_coordinates_ = true;
  cv::Point rclick_;
  // SingleObjViz single_obj_;
  // std::vector<ObjId> chosen_objs_;
  std::vector<cv::Point2d> stop_line_start_points_;
  cv::Point2i ruler_head_;
  cv::Point2i ruler_tail_;
  cv::Point2i mouse_position_;
  std::vector<cv::Point2i> log_polygon_;
  bool logging_ = false;
  // std::string log_file_path_ = "/home/caros/tmp/";
  // std::unordered_map<
  //     int,
  //     std::pair<std::unique_ptr<std::ofstream>,
  //               std::unique_ptr<google::protobuf::io::OstreamOutputStream>>>
  //     log_file_id_;

  cv::Mat image_;
  // MapInfo* map_ = nullptr;
  // DashBoard* dashboard_;
};

}  // namespace msf
}  // namespace perception
}  // namespace airos
