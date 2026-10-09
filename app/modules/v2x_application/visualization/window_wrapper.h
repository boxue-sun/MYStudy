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

#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "air_service/framework/proto/airos_perception_obstacle.pb.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "app/modules/v2x_application/proto/roi_lane.pb.h"
#include "app/modules/v2x_application/proto/roi_points.pb.h"

#include "app/modules/v2x_application/usecase/base/config_manager.h"
#include "app/modules/v2x_application/usecase/base/runner.h"
#include "app/modules/v2x_application/usecase/common/factory.hpp"
#include "app/modules/v2x_application/usecase/common/vec2d.h"
#include "base/common/log.h"
#include "opencv2/core/types.hpp"
#include "opencv2/opencv.hpp"
#include "yaml-cpp/yaml.h"

namespace airos {
namespace perception {
namespace usecase {

class WindowWrapper {
 public:
  WindowWrapper()  = default;
  ~WindowWrapper() = default;
  void Init();

  void CalculateScale(const std::shared_ptr<airos::usecase::EventOutputResult>&
                          event_output_result);
  void DrawObstacle(const std::shared_ptr<airos::usecase::EventOutputResult>&
                        event_output_result);
  void Show();
  void KeyCallbackFunc(int key);
  static void MouseCallBackFunc(
      int event, int x, int y, int flags, void* userdata);
  static void ModeChangeCallBackFunc(int number, void* usedata);
  void SetManual() {
    manual_ = true;
  }
  void ResetBackground() {
    image_ = cv::Mat(height_, width_, CV_8UC3, cv::Scalar::all(255));
  }

  void AddRuler(cv::Point2i point, bool head);

  std::string GetPercepitonType(
      const airos::perception::PerceptionObstacle& object);
  void DrawText(
      const std::string& text, const cv::Point2f& point,
      const cv::Scalar& color);
  void DrawPoint(const cv::Point2f& point);
  bool GetStaticPoly(const std::string& nm, const std::string& pt_path);

  bool GetText();
  bool AddHighLightObj(int64_t obj_id);
  bool AddHighLightLaneId(std::string lane_id);
  bool GetHighLightObj(std::unordered_set<int64_t>* objs);
  bool GetHighLightLaneId(std::unordered_set<std::string>* lane_ids);
  bool DrawHighLightObj(const airos::perception::PerceptionObstacle& object);
  bool DrawHighLightObj(
      const airos::perception::PerceptionObstacle& object,
      const cv::Scalar& color);

 public:
  int lane_congestion_         = 0;
  int id_switch_               = 0;
  int type_switch_             = 0;
  int draw_get_text_           = 0;
  int draw_polygon_            = 0;
  int draw_lane_and_direction_ = 0;

  double lane_radius_ = 0;
  bool is_map_drawed_ = false;

  std::vector<cv::Point2f> lane_left_boundary;
  std::vector<cv::Point2f> lane_right_boundary;

  cv::Point2f TranslatePosition(const cv::Point2f& point);
  void DrawLane(
      const std::vector<cv::Point2d>& vertices, const cv::Scalar& color,
      int thickness = 1);
  void DrawPolygon(
      const std::vector<cv::Point2d>& vertices, const cv::Scalar& color,
      int thickness = 1);
  std::map<
      std::string, std::vector<std::vector<airos::perception::usecase::Vec2d>>>
      static_polygon_;
  std::map<std::string, cv::Point2f> front_cam_pos_;

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
  void SetMousePos(cv::Point2i point);
  void ZoomOut();
  cv::Point2f GetCenter();
  cv::Point2f GetTopLeft();
  void DrawPolygon(const cv::Scalar& color);
  void DrawSpeed(
      const airos::perception::PerceptionObstacle& object,
      const cv::Point2f& point1, const cv::Scalar& color);

  void GetObjectPolygon(const airos::perception::PerceptionObstacle& object);
  cv::Point2f InverseTranslatePosition(const cv::Point2f& point);
  cv::Point2d TranslatePosition2d(const cv::Point2d& point);
  void MakePolygon(cv::Point2f point, int flags, int event);
  void ReadRoiPolygon(std::string roi_path);  // read bin/target.pt

  void MakeLanePolygon(cv::Point2f point, int flags, int event);
  void ReadLanePolygon(std::string roi_path);
  void ReadLaneFromCyber(std::string roi_path);
  void DrawLaneDirection(
      const std::vector<cv::Point2d>& vertices, const cv::Scalar& color,
      int thickness = 1);
  bool SaveLaneParamFile();

 public:
  bool get_text_doing = false;

 public:
  // for show polygon in viz
  void ShowMakePolygon();
  void ShowMakeLanePolygon();
  void DrawLanePolygon();

  void DrawLaneDirection();

 private:
  const double to_degree_ = 180.0 / M_PI;
  const float kscale_id_  = 14;
  std::vector<cv::Point2f> vertices_;
  // image coordinates
  int height_          = 1080;
  int width_           = 1080;
  double border_x_min_ = 1e9;
  double border_y_min_ = 1e9;
  double border_x_max_ = 0;
  double border_y_max_ = 0;
  // tansition
  float scale_ = 1;
  bool manual_ = false;
  cv::Point rclick_;
  cv::Point2i ruler_head_;
  cv::Point2i ruler_tail_;

  std::mutex border_mutex_;
  cv::Point2i mouse_position_;

  // for show high light road boundary and high light obj
  std::mutex high_light_mutex_;
  std::unordered_set<int64_t> high_light_obj_;
  std::unordered_set<std::string> high_light_lane_id_;
  // for make polygon in viz
  std::vector<std::vector<cv::Point2d>> make_polygons_;
  int make_polygon_index_ = 0;

  // for make lane in viz
  std::vector<std::pair<std::vector<cv::Point2d>, std::vector<cv::Point2d>>>
      make_lane_polygons_;  // polygon—direction
  int make_lane_polygon_index_ = 0;

  std::vector<std::pair<std::vector<cv::Point2d>, std::vector<cv::Point2d>>>
      lane_polygon_from_cyber_;  // lane from cyber

  cv::Mat image_;
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
