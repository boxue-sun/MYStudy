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

#include "app/modules/v2x_application/visualization/window_wrapper.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "app/modules/v2x_application/usecase/common/util.h"
#include "base/common/singleton.h"
#include "base/io/protobuf_util.h"

namespace airos {
namespace perception {
namespace usecase {

void WindowWrapper::Init() {
  cv::namedWindow("Button", cv::WINDOW_AUTOSIZE);
  cv::namedWindow("UseCase");
  cv::createTrackbar("lane", "Button", &draw_lane_and_direction_, 3);
  cv::createTrackbar("lane_congestion", "Button", &lane_congestion_, 1);
  cv::createTrackbar("perception_id", "Button", &id_switch_, 1);
  cv::createTrackbar("perception_type", "Button", &type_switch_, 1);
  cv::createTrackbar("high_light_obj", "Button", &draw_get_text_, 1);
  cv::createTrackbar("draw_polygon", "Button", &draw_polygon_, 2);

  cv::setTrackbarPos("lane", "Button", 1);
  cv::setTrackbarPos("lane_congestion", "Button", 0);
  cv::setTrackbarPos("perception_id", "Button", 1);
  cv::setTrackbarPos("perception_type", "Button", 0);
  cv::setTrackbarPos("high_light_obj", "Button", 0);
  cv::setTrackbarPos("draw_polygon", "Button", 0);
  // set the callback function for any mouse event
  cv::setMouseCallback("UseCase", MouseCallBackFunc, this);

  auto mgr   = base::Singleton<ConfigManager>::get_instance();
  auto param = Factory<BaseParams>::Instance().GetShared("usecase");
  mgr->LoadFor(param);
  auto lane_param_path_ = param->GetVal("lane_param_path").Cast<std::string>();
  ReadLaneFromCyber(lane_param_path_);

  std::string file_path =
      "/home/airos/os/app/conf/airos_v2x_application/conf/target.pt";
  std::ifstream file_target(file_path.c_str());
  if (file_target.good()) {
    ReadRoiPolygon(file_path);
  } else {
    APP_LOG_WARN << "Load target.pt failed, try next.";
  }

  file_path =
      "/home/airos/os/app/conf/airos_v2x_application/conf/lane_param.pt";
  std::ifstream file_lane(file_path.c_str());
  if (file_lane.good()) {
    ReadLanePolygon(file_path);
  } else {
    APP_LOG_WARN << "Load lane_param.pt failed, try next.";
  }
}

bool WindowWrapper::GetStaticPoly(
    const std::string& nm, const std::string& pt_path) {
  airos::usecase::roi::RoiParams roi_params_;
  if (!airos::base::ParseProtobufFromFile(pt_path, &roi_params_)) {
    APP_LOG_ERROR << "Parse Crosswalk and bicycle From File Failed";
    return false;
  }
  std::vector<airos::perception::usecase::Vec2d> roi_poly;
  std::vector<std::vector<airos::perception::usecase::Vec2d>> polygons;
  for (const auto& single_camera_roi : roi_params_.roi_params()) {
    roi_poly.clear();
    for (const auto& point : single_camera_roi.polygon()) {
      airos::perception::usecase::Vec2d point2d;
      point2d.SetX(point.x());
      point2d.SetY(point.y());
      roi_poly.push_back(point2d);
    }
    polygons.push_back(roi_poly);
  }
  if (!polygons.empty()) {
    static_polygon_[nm] = polygons;
  }
  return false;
  return true;
}

void WindowWrapper::CalculateScale(
    const std::shared_ptr<airos::usecase::EventOutputResult>&
        event_output_result) {
  if (!manual_) {
    for (int i = 0; i < event_output_result->perception_obstacle_size(); ++i) {
      const airos::perception::PerceptionObstacle object =
          event_output_result->perception_obstacle(i);
      cv::Point2d position(object.position().x(), object.position().y());
      border_x_min_ = position.x < border_x_min_ ? position.x : border_x_min_;
      border_x_max_ = position.x > border_x_max_ ? position.x : border_x_max_;
      border_y_min_ = position.y < border_y_min_ ? position.y : border_y_min_;
      border_y_max_ = position.y > border_y_max_ ? position.y : border_y_max_;

      double cur_w = border_x_max_ - border_x_min_;
      double cur_h = border_y_max_ - border_y_min_;
      if (cur_w < cur_h) {
        border_x_min_ = border_x_min_ - (cur_h - cur_w) / 2;
        border_x_max_ = border_x_max_ + (cur_h - cur_w) / 2;
      } else {
        border_y_min_ = border_y_min_ + (cur_h - cur_w) / 2;
        border_y_max_ = border_y_max_ - (cur_h - cur_w) / 2;
      }
      // border_y_max_ = border_y_min_ + (border_x_max_ - border_x_min_);
    }
    scale_ = width_ / (border_x_max_ - border_x_min_);
  }
}

void WindowWrapper::DrawObstacle(
    const std::shared_ptr<airos::usecase::EventOutputResult>&
        event_output_result) {
  std::unordered_set<int64_t> high_light_objs;
  GetHighLightObj(&high_light_objs);

  for (const auto& object : event_output_result->perception_obstacle()) {
    GetObjectPolygon(object);
    cv::Scalar color;
    color = cv::Scalar(0, 255, 0);
    if (object.type() == airos::perception::PerceptionObstacle::VEHICLE) {
      color = cv::Scalar(255, 0, 0);
    }
    if (object.sub_type() == airos::perception::SubType::TRAFFICCONE) {
      color = cv::Scalar(0, 0, 255);
    }
    DrawPolygon(color);
    cv::Point2f center = GetCenter();
    DrawSpeed(object, center, color);
    color = cv::Scalar(0, 0, 0);

    std::string show_text = "";
    if (id_switch_) show_text += std::to_string(object.id());
    if (type_switch_) show_text += "-" + GetPercepitonType(object);

    if (show_text[0] == '-') {
      show_text.erase(show_text.begin());
    }
    if (show_text.find("-") == std::string::npos) {
      color = cv::Scalar(0, 0, 0);
    } else {
      color = cv::Scalar(0, 0, 255);
    }
    DrawText(show_text, center, color);

    int64_t obj_id = object.id();
    if (high_light_objs.count(obj_id)) {
      DrawHighLightObj(object);
    }
  }

  return;
}

void WindowWrapper::DrawText(
    const std::string& text, const cv::Point2f& point,
    const cv::Scalar& color) {
  cv::putText(
      image_, text, point, cv::FONT_HERSHEY_SIMPLEX, scale_ / kscale_id_, color,
      1);
}

void WindowWrapper::DrawPoint(const cv::Point2f& point) {
  cv::circle(image_, point, 2, cv::Scalar(0, 0, 255), -1);
}

std::string WindowWrapper::GetPercepitonType(
    const airos::perception::PerceptionObstacle& object) {
  std::string sub_type = "";
  switch (object.sub_type()) {
    case airos::perception::UNKNOWN:
      sub_type = "Unknown";
      break;

    case airos::perception::UNKNOWN_MOVABLE:
      sub_type = "Unknown_movable";
      break;

    case airos::perception::CAR:
      sub_type = "Car";
      break;

    case airos::perception::VAN:
      sub_type = "Van";
      break;

    case airos::perception::TRUCK:
      sub_type = "Truck";
      break;

    case airos::perception::BUS:
      sub_type = "Bus";
      break;

    case airos::perception::CYCLIST:
      sub_type = "Cyclist";
      break;

    case airos::perception::MOTORCYCLIST:
      sub_type = "Motorcyclist";
      break;

    case airos::perception::TRICYCLIST:
      sub_type = "Tricyclist";
      break;

    case airos::perception::PEDESTRIAN:
      sub_type = "Pedestrian";
      break;

    case airos::perception::TRAFFICCONE:
      sub_type = "Trafficcone";
      break;

    case airos::perception::SAFETY_TRIANGLE:
      sub_type = "Safety Triangle";
      break;

    default:
      break;
  }
  return sub_type;
}

cv::Point2f WindowWrapper::GetCenter() {
  cv::Point2f center;
  center.x =
      (vertices_[0].x + vertices_[1].x + vertices_[2].x + vertices_[3].x) / 4;
  center.y =
      (vertices_[0].y + vertices_[1].y + vertices_[2].y + vertices_[3].y) / 4;

  return center;
}

cv::Point2f WindowWrapper::GetTopLeft() {
  cv::Point2f top_left;
  for (int i = 0; i < 4; ++i) {
    if (vertices_[i].y < vertices_[(i + 1) % vertices_.size()].y) {
      top_left.y = vertices_[i].y;
    }
    if (vertices_[i].x < vertices_[(i + 1) % vertices_.size()].x) {
      top_left.x = vertices_[i].x;
    }
  }

  return top_left;
}

void WindowWrapper::DrawPolygon(const cv::Scalar& color) {
  for (size_t i = 0; i < vertices_.size(); i++) {
    cv::line(
        image_, vertices_[i], vertices_[(i + 1) % vertices_.size()], color, 2,
        cv::LINE_AA);
  }
}

void WindowWrapper::DrawLane(
    const std::vector<cv::Point2d>& vertices, const cv::Scalar& color,
    int thickness) {
  for (size_t i = 0; i < vertices.size() - 1; i++) {
    cv::line(
        image_, vertices[i], vertices[(i + 1) % vertices.size()], color,
        thickness);
  }
}

void WindowWrapper::DrawPolygon(
    const std::vector<cv::Point2d>& vertices, const cv::Scalar& color,
    int thickness) {
  for (size_t i = 0; i < vertices.size(); i++) {
    cv::line(
        image_, vertices[i], vertices[(i + 1) % vertices.size()], color,
        thickness);
  }
}

void WindowWrapper::DrawSpeed(
    const airos::perception::PerceptionObstacle& object,
    const cv::Point2f& point1, const cv::Scalar& color) {
  cv::Point2f point2;
  point2.x = point1.x + object.velocity().x() * scale_;
  // the y axis in reversed int image coordinates
  point2.y = point1.y - object.velocity().y() * scale_;
  cv::line(image_, point1, point2, color, 2, cv::LINE_AA);
}

void WindowWrapper::GetObjectPolygon(
    const airos::perception::PerceptionObstacle& object) {
  vertices_.clear();
  cv::Point2d position(object.position().x(), object.position().y());
  cv::Size2f size(object.length(), object.width());
  cv::RotatedRect rRect = cv::RotatedRect(
      position, size,
      object.theta() * to_degree_);  // theta may wrong

  cv::Point2f vertices[4];
  rRect.points(vertices);

  // translate into image coordinates
  for (int i = 0; i < 4; ++i) {
    vertices_.push_back(TranslatePosition(vertices[i]));
  }

  return;
}

cv::Point2f WindowWrapper::TranslatePosition(const cv::Point2f& point) {
  cv::Point2f trans_point;
  trans_point.x = (point.x - border_x_min_) * scale_;
  trans_point.y = height_ - (point.y - border_y_min_) * scale_;
  return trans_point;
}

cv::Point2f WindowWrapper::InverseTranslatePosition(const cv::Point2f& point) {
  cv::Point2f trans_point;
  trans_point.x = point.x / scale_ + border_x_min_;
  trans_point.y = (height_ - point.y) / scale_ + border_y_min_;
  return trans_point;
}

cv::Point2d WindowWrapper::TranslatePosition2d(const cv::Point2d& point) {
  cv::Point2d trans_point;
  trans_point.x = (point.x - border_x_min_) * scale_;
  trans_point.y = height_ - (point.y - border_y_min_) * scale_;
  return trans_point;
}

void WindowWrapper::Show() {
  // show the image
  // cv::putText(image_, std::to_string(border_x_min_), cv::Point2f(5, 50),
  //             cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);

  if (ruler_head_.x != ruler_tail_.x || ruler_head_.y != ruler_tail_.y) {
    cv::line(
        image_, ruler_head_, ruler_tail_, cv::Scalar(0, 0, 255), 2,
        cv::LINE_AA);
    cv::putText(
        image_,
        std::to_string(cv::norm(
            InverseTranslatePosition(ruler_tail_) -
            InverseTranslatePosition(ruler_head_))),
        ruler_tail_, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
  }
  cv::imshow("UseCase", image_);
}

void WindowWrapper::AddRuler(cv::Point2i point, bool head) {
  if (head) {
    ruler_head_ = point;
  } else {
    ruler_tail_ = point;
  }
}

void WindowWrapper::SetMousePos(cv::Point2i point) {
  mouse_position_ = point;
}

void WindowWrapper::MouseCallBackFunc(
    int event, int x, int y, int flags, void* userdata) {
  static int drag_start_x = 0;
  static int drag_start_y = 0;
  WindowWrapper* pThis    = reinterpret_cast<WindowWrapper*>(userdata);
  if (flags) {
    // std::cout << "flags = " << flags << std::endl;
  }

  if (event == cv::EVENT_LBUTTONDOWN) {
    pThis->SetManual();
    drag_start_x = x;
    drag_start_y = y;
    pThis->SetMousePos(cv::Point2i(x, y));
  } else if (event == cv::EVENT_LBUTTONUP) {
  } else if (
      event == cv::EVENT_MOUSEMOVE && flags % 32 == cv::EVENT_FLAG_LBUTTON) {
    int diff_x = x - drag_start_x;
    int diff_y = y - drag_start_y;

    pThis->Drag(diff_x, diff_y);

    drag_start_x = x;
    drag_start_y = y;
  } else if (event == cv::EVENT_RBUTTONDOWN) {
    pThis->AddRuler(cv::Point2i(x, y), true);
    pThis->AddRuler(cv::Point2i(x, y), false);
  } else if (
      event == cv::EVENT_MOUSEMOVE && flags % 32 == cv::EVENT_FLAG_RBUTTON) {
    // add ruler tools
    pThis->AddRuler(cv::Point2i(x, y), false);
  } else if (event == cv::EVENT_RBUTTONUP) {
    double dis = cv::norm(
        pThis->InverseTranslatePosition(pThis->ruler_tail_) -
        pThis->InverseTranslatePosition(pThis->ruler_head_));
    if (dis > 0.1) {
      cv::Point2f point = pThis->InverseTranslatePosition(pThis->ruler_head_);
      std::cout << std::setprecision(16) << "head: [" << point.x << ", "
                << point.y << "], ";
      point = pThis->InverseTranslatePosition(pThis->ruler_tail_);
      std::cout << std::setprecision(16) << "tail: [" << point.x << ","
                << point.y << "], ";
      std::cout << "distance: " << dis << std::endl;
    }
    pThis->AddRuler(cv::Point2i(0, 0), true);
    pThis->AddRuler(cv::Point2i(0, 0), false);
  } else if (event == cv::EVENT_MOUSEWHEEL) {
    pThis->SetManual();
    if (cv::getMouseWheelDelta(flags) < 0) {
      pThis->ZoomIn();
    } else {
      pThis->ZoomOut();
    }
  } else if (event == cv::EVENT_RBUTTONDBLCLK) {
    pThis->rclick_.x = x;
    pThis->rclick_.y = y;
  }

  if (pThis->draw_polygon_ == 1) {
    pThis->MakePolygon(cv::Point2f(x, y), flags, event);
  } else if (pThis->draw_polygon_ == 2) {
    pThis->MakeLanePolygon(cv::Point2f(x, y), flags, event);
  }
}

void WindowWrapper::MakeLanePolygon(cv::Point2f point, int flags, int event) {
  point = InverseTranslatePosition(point);
  enum Action {
    UNKNOWN                  = 0,
    ADD_LANE_POLYGON_POINT   = 1,
    DEL_LANE_POLYGON_POINT   = 2,
    ADD_LANE_DIRECTION_POINT = 3,
    DEL_LANE_DIRECTION_POINT = 4,
    ADD_LANE                 = 5,
    CHANGE_INDEX             = 6,
    SAVE_FILE                = 7,
  };
  Action now_action = UNKNOWN;
  if (event == cv::EVENT_LBUTTONDOWN && flags % 32 == cv::EVENT_FLAG_CTRLKEY) {
    now_action = ADD_LANE_POLYGON_POINT;
  } else if (
      event == cv::EVENT_RBUTTONDOWN && flags % 32 == cv::EVENT_FLAG_CTRLKEY) {
    now_action = DEL_LANE_POLYGON_POINT;
  } else if (
      event == cv::EVENT_LBUTTONDOWN && flags % 32 == cv::EVENT_FLAG_SHIFTKEY) {
    now_action = ADD_LANE_DIRECTION_POINT;
  } else if (
      event == cv::EVENT_RBUTTONDOWN && flags % 32 == cv::EVENT_FLAG_SHIFTKEY) {
    now_action = DEL_LANE_DIRECTION_POINT;
  } else if (
      event == cv::EVENT_LBUTTONDOWN &&
      flags % 32 == cv::EVENT_FLAG_CTRLKEY + cv::EVENT_FLAG_SHIFTKEY) {
    now_action = ADD_LANE;
  } else if (
      event == cv::EVENT_RBUTTONDOWN &&
      flags % 32 == cv::EVENT_FLAG_CTRLKEY + cv::EVENT_FLAG_SHIFTKEY) {
    now_action = SAVE_FILE;
  } else if (flags % 32 == cv::EVENT_FLAG_MBUTTON + cv::EVENT_FLAG_CTRLKEY) {
    now_action = CHANGE_INDEX;
  }

  switch (now_action) {
    case ADD_LANE_POLYGON_POINT:
      std::cout << "ADD_LANE_POLYGON_POINT" << std::endl;
      if (make_lane_polygons_.size() == 0) {
        make_lane_polygons_.push_back({{}, {}});
      }
      make_lane_polygon_index_ %= make_lane_polygons_.size();
      make_lane_polygons_[make_lane_polygon_index_].first.push_back(point);
      std::cout << "index = " << make_lane_polygon_index_ << std::endl;
      break;
    case DEL_LANE_POLYGON_POINT:
      std::cout << "DEL_LANE_POLYGON_POINT" << std::endl;
      if (make_lane_polygons_.size() != 0) {
        if (make_lane_polygons_[make_lane_polygon_index_].first.size() != 0) {
          make_lane_polygons_[make_lane_polygon_index_].first.pop_back();
        }
      }
      break;
    case ADD_LANE_DIRECTION_POINT:
      std::cout << "ADD_LANE_DIRECTION_POINT" << std::endl;
      if (make_lane_polygons_.size() == 0) {
        make_lane_polygons_.push_back({{}, {}});
      }
      make_lane_polygon_index_ %= make_lane_polygons_.size();
      make_lane_polygons_[make_lane_polygon_index_].second.push_back(point);
      std::cout << "index = " << make_lane_polygon_index_ << std::endl;
      break;
    case DEL_LANE_DIRECTION_POINT:
      std::cout << "DEL_LANE_DIRECTION_POINT" << std::endl;
      if (make_lane_polygons_.size() != 0) {
        if (make_lane_polygons_[make_lane_polygon_index_].second.size() != 0) {
          make_lane_polygons_[make_lane_polygon_index_].second.pop_back();
        }
      }
      break;
    case ADD_LANE:
      std::cout << "ADD_LANE" << std::endl;
      make_lane_polygons_.push_back({{}, {}});
      make_lane_polygon_index_ = make_lane_polygons_.size() - 1;
      break;
    case CHANGE_INDEX:
      make_lane_polygon_index_ += 1;
      if (make_lane_polygons_.size() != 0) {
        make_lane_polygon_index_ %= make_lane_polygons_.size();
      } else {
        make_lane_polygon_index_ = 0;
      }
      break;
    case SAVE_FILE:
      std::cout << "SAVE_FILE" << std::endl;
      if (!SaveLaneParamFile()) {
        APP_LOG_ERROR << "Failed to save lane_param.pt .\n";
      }
      break;
    default:
      break;
  }
  // 处理所有的空数据
  for (auto iter = make_lane_polygons_.begin();
       iter != make_lane_polygons_.end();) {
    if (iter->first.size() == 0 && iter->second.size() == 0) {
      std::cout << "first and second null, delete it" << std::endl;
      iter = make_lane_polygons_.erase(iter);
    } else {
      iter++;
    }
    if (iter + 1 == make_lane_polygons_.end()) {
      break;
    }
  }
  if (make_lane_polygons_.size() != 0) {
    make_lane_polygon_index_ %= make_lane_polygons_.size();
  }
}
bool WindowWrapper::SaveLaneParamFile() {
  airos::usecase::roi::LaneParam lane_params;
  int lane_count = 0;
  for (auto lane_roi : make_lane_polygons_) {
    auto lane_polygon   = lane_roi.first;
    auto lane_direction = lane_roi.second;
    if (lane_polygon.size() <= 2 || lane_direction.size() <= 1) {
      continue;
    }
    auto lane_roi_param = lane_params.add_lane_roi_param();
    for (auto& pt : lane_polygon) {
      auto p = lane_roi_param->add_polygon();
      p->set_x(pt.x);
      p->set_y(pt.y);
    }
    for (auto& pt : lane_direction) {
      auto center = lane_roi_param->add_center();
      center->set_x(pt.x);
      center->set_y(pt.y);
    }
    lane_roi_param->set_lane_id(std::to_string(++lane_count));
  }
  lane_params.set_rscu_id("");
  lane_params.mutable_feature_position()->set_x(0);
  lane_params.mutable_feature_position()->set_y(0);
  lane_params.mutable_feature_position()->set_z(0);

  std::string str_proto;
  google::protobuf::TextFormat::PrintToString(lane_params, &str_proto);
  std::cout << str_proto << std::endl;
  try {
    std::fstream fout(
        "/home/airos/os/app/conf/airos_v2x_application/conf/"
        "lane_param.pt",
        std::ios::out | std::ios::trunc);
    fout << str_proto << std::endl;
    fout.close();
  } catch (std::exception& e) {
    APP_LOG_ERROR << "Failed to save lane_param.pt .\n";
    return false;
  }
  return true;
}

// read write
void WindowWrapper::ReadLanePolygon(std::string roi_path) {
  make_lane_polygons_.clear();
  airos::usecase::roi::LaneParam lane_params;

  if (!airos::base::ParseProtobufFromFile(roi_path, &lane_params)) {
    APP_LOG_ERROR << "ReadLanePolygon : Parse Protobuf From File Failed";
    return;
  }
  for (auto& lane_roi : lane_params.lane_roi_param()) {
    std::vector<cv::Point2d> lane_polygon;
    std::vector<cv::Point2d> lane_direction;

    for (auto& point : lane_roi.polygon()) {
      cv::Point2d pt(point.x(), point.y());
      lane_polygon.push_back(pt);
    }
    for (auto& point : lane_roi.center()) {
      cv::Point2d pt(point.x(), point.y());
      lane_direction.push_back(pt);
    }
    std::string lane_id = lane_roi.lane_id();
    if (lane_polygon.size() != 0 || lane_direction.size() != 0) {
      make_lane_polygons_.push_back({lane_polygon, lane_direction});
    }
  }
}

// read only
void WindowWrapper::ReadLaneFromCyber(std::string roi_path) {
  lane_polygon_from_cyber_.clear();
  airos::usecase::roi::LaneParam lane_params;

  if (!airos::base::ParseProtobufFromFile(roi_path, &lane_params)) {
    APP_LOG_ERROR << "ReadLanePolygon : Parse Protobuf From File Failed";
    return;
  }
  for (auto& lane_roi : lane_params.lane_roi_param()) {
    std::vector<cv::Point2d> lane_polygon;
    std::vector<cv::Point2d> lane_direction;

    for (auto& point : lane_roi.polygon()) {
      cv::Point2d pt(point.x(), point.y());
      lane_polygon.push_back(pt);
    }
    for (auto& point : lane_roi.center()) {
      cv::Point2d pt(point.x(), point.y());
      lane_direction.push_back(pt);
    }
    std::string lane_id = lane_roi.lane_id();
    if (lane_polygon.size() != 0 || lane_direction.size() != 0) {
      lane_polygon_from_cyber_.push_back({lane_polygon, lane_direction});
    }
  }
}

void WindowWrapper::ShowMakeLanePolygon() {
  for (int i = 0, size = make_lane_polygons_.size(); i < size; i++) {
    auto lane_roi = make_lane_polygons_[i];
    if (lane_roi.first.size() == 0 && lane_roi.second.size() == 0) {
      continue;
    }
    std::vector<cv::Point2d> lane_polygon;
    std::vector<cv::Point2d> lane_direction;
    for (auto pt : lane_roi.first) {
      pt = TranslatePosition(pt);
      lane_polygon.push_back(pt);
    }
    for (auto pt : lane_roi.second) {
      pt = TranslatePosition(pt);
      lane_direction.push_back(pt);
    }
    if (i == make_lane_polygon_index_) {
      DrawPolygon(lane_polygon, cv::Scalar(0, 0, 255), 2);
      DrawLaneDirection(lane_direction, cv::Scalar(255, 0, 0), 2);

    } else {
      DrawPolygon(lane_polygon, cv::Scalar(102, 51, 17), 2);
      DrawLaneDirection(lane_direction, cv::Scalar(102, 51, 17), 2);
    }
  }
}

void WindowWrapper::DrawLaneDirection(
    const std::vector<cv::Point2d>& vertices, const cv::Scalar& color,
    int thickness) {
  if (vertices.empty()) {
    return;
  }
  DrawLane(vertices, color, thickness);
  auto point = vertices.back();
  cv::circle(image_, point, 5, cv::Scalar(87, 201, 0), 4);
}

void WindowWrapper::DrawLanePolygon() {
  for (int i = 0, size = lane_polygon_from_cyber_.size(); i < size; i++) {
    auto lane_roi = lane_polygon_from_cyber_[i];
    if (lane_roi.first.size() == 0 && lane_roi.second.size() == 0) {
      continue;
    }
    std::vector<cv::Point2d> lane_polygon;
    for (auto pt : lane_roi.first) {
      pt = TranslatePosition(pt);
      lane_polygon.push_back(pt);
    }
    DrawPolygon(lane_polygon, cv::Scalar(102, 51, 17), 2);
  }
}

void WindowWrapper::DrawLaneDirection() {
  for (int i = 0, size = lane_polygon_from_cyber_.size(); i < size; i++) {
    auto lane_roi = lane_polygon_from_cyber_[i];
    if (lane_roi.first.size() == 0 && lane_roi.second.size() == 0) {
      continue;
    }
    std::vector<cv::Point2d> lane_direction;
    for (auto pt : lane_roi.second) {
      pt = TranslatePosition(pt);
      lane_direction.push_back(pt);
    }
    DrawLaneDirection(lane_direction, cv::Scalar(102, 51, 17), 2);
  }
}

void WindowWrapper::MakePolygon(cv::Point2f point, int flags, int event) {
  point = InverseTranslatePosition(point);
  if (event == cv::EVENT_LBUTTONDOWN && flags % 32 == cv::EVENT_FLAG_CTRLKEY) {
    // 画点
    std::cout << "draw point" << std::endl;
    if (make_polygons_.size() == 0) {
      make_polygons_.push_back({});
    }
    make_polygon_index_ %= make_polygons_.size();
    make_polygons_[make_polygon_index_].push_back(point);
    std::cout << "index = " << make_polygon_index_ << std::endl;
  } else if (
      event == cv::EVENT_RBUTTONDOWN && flags % 32 == cv::EVENT_FLAG_CTRLKEY) {
    // 删除点
    std::cout << "del point" << std::endl;

    if (make_polygons_.size() != 0) {
      if (make_polygons_[make_polygon_index_].size() != 0) {
        make_polygons_[make_polygon_index_].pop_back();
      }
    }
  } else if (
      event == cv::EVENT_LBUTTONDOWN &&
      flags % 32 == cv::EVENT_FLAG_CTRLKEY + cv::EVENT_FLAG_SHIFTKEY) {
    // 新增polygon
    std::cout << "add polygon" << std::endl;
    make_polygons_.push_back({});
    make_polygon_index_ = make_polygons_.size() - 1;
  } else if (
      event == cv::EVENT_RBUTTONDOWN &&
      flags % 32 == cv::EVENT_FLAG_CTRLKEY + CV_EVENT_FLAG_SHIFTKEY) {
    // 保存文件
    std::cout << "save polygon" << std::endl;

    airos::usecase::roi::RoiParams roi_params;
    for (auto& polygons : make_polygons_) {
      if (polygons.size() <= 2) {
        continue;
      }
      auto singleroi_param = roi_params.add_roi_params();
      std::cout << "set polygon" << std::endl;
      for (auto& pt : polygons) {
        auto p = singleroi_param->add_polygon();
        p->set_x(pt.x);
        p->set_y(pt.y);
        std::cout << "set point " << pt.x << " " << pt.y << std::endl;
      }
      singleroi_param->set_camera_id(1);
    }
    std::string str_proto;
    google::protobuf::TextFormat::PrintToString(roi_params, &str_proto);
    std::cout << str_proto << std::endl;

    try {
      std::fstream fout(
          "/home/airos/os/app/conf/airos_v2x_application/conf/target.pt",
          std::ios::out | std::ios::trunc);
      fout << str_proto << std::endl;
      fout.close();
    } catch (std::exception& e) {
      std::cout << "save error " << e.what() << std::endl;
      return;
    }

    std::cout << "save over" << std::endl;
  } else if (
      event == cv::EVENT_LBUTTONDOWN && flags % 32 == cv::EVENT_FLAG_SHIFTKEY) {
    // 切换index
    make_polygon_index_ += 1;
    int size = make_polygons_.size();
    if (size != 0) {
      make_polygon_index_ %= size;
    } else {
      make_polygon_index_ = 0;
    }
  }
  // 处理所有的空数据
  for (auto iter = make_polygons_.begin(); iter != make_polygons_.end();) {
    if (iter->size() == 0) {
      iter = make_polygons_.erase(iter);
    } else {
      iter++;
    }
    if (iter + 1 == make_polygons_.end()) {
      break;
    }
  }
  if (make_polygons_.size() != 0) make_polygon_index_ %= make_polygons_.size();
}

void WindowWrapper::ReadRoiPolygon(std::string roi_path) {
  make_polygons_.clear();
  airos::usecase::roi::RoiParams roi_params;
  if (!airos::base::ParseProtobufFromFile(roi_path, &roi_params)) {
    APP_LOG_ERROR << "read roi polygon : Parse Protobuf From File Failed";
    return;
  }
  for (auto& single_camera_roi : roi_params.roi_params()) {
    std::vector<cv::Point2d> vec;
    for (auto& point : single_camera_roi.polygon()) {
      cv::Point2d pt(point.x(), point.y());
      vec.push_back(pt);
    }
    if (vec.size() != 0) {
      make_polygons_.push_back(vec);
    }
  }
}

void WindowWrapper::ShowMakePolygon() {
  for (int i = 0, size = make_polygons_.size(); i < size; i++) {
    auto polygons = make_polygons_[i];
    if (polygons.size() < 2) {
      continue;
    }
    std::vector<cv::Point2d> vec;
    for (auto pt : polygons) {
      pt = TranslatePosition(pt);
      vec.push_back(pt);
    }
    if (i == make_polygon_index_) {
      DrawPolygon(vec, cv::Scalar(102, 51, 17), 2);
    } else {
      DrawPolygon(vec, cv::Scalar(0, 0, 255), 2);
    }
  }
}

void WindowWrapper::KeyCallbackFunc(int key) {
  switch (key) {
    case 'y':
      manual_ = false;
      // RestBorder();
      break;
    case 'w':
    case 's':
    case 'a':
    case 'd':
      manual_ = true;
      Drag(key);
      break;
    case 'z':
    case 'c':
      manual_ = true;
      Zoom(key);
  }
}

void WindowWrapper::Drag(int key) {
  switch (key) {
    case 'w':
      DragUp();
      break;
    case 's':
      DragDown();
      break;
    case 'a':
      DragLeft();
      break;
    case 'd':
      DragRight();
      break;
  }
}

void WindowWrapper::Drag(int x, int y) {
  border_x_max_ -= x / scale_;
  border_y_max_ += y / scale_;
  border_x_min_ -= x / scale_;
  border_y_min_ += y / scale_;
}

void WindowWrapper::DragUp() {
  // border_x_max_ += 10;
  border_y_max_ += 10;
  // border_x_min_ += 10;
  border_y_min_ += 10;
}

void WindowWrapper::DragDown() {
  // border_x_max_ += 10;
  border_y_max_ -= 10;
  // border_x_min_ += 10;
  border_y_min_ -= 10;
}

void WindowWrapper::DragLeft() {
  border_x_max_ -= 10;
  // border_y_max_ += 10;
  border_x_min_ -= 10;
  // border_y_min_ += 10;
}

void WindowWrapper::DragRight() {
  border_x_max_ += 10;
  // border_y_max_ += 10;
  border_x_min_ += 10;
  // border_y_min_ += 10;
}
void WindowWrapper::Zoom(int key) {
  switch (key) {
    case 'z':
      ZoomIn();
      break;
    case 'c':
      ZoomOut();
      break;
  }
}

void WindowWrapper::ZoomIn() {
  std::lock_guard<std::mutex> lck(border_mutex_);
  cv::Point2d center = InverseTranslatePosition(mouse_position_);

  double diff_x   = center.x - (border_x_max_ + border_x_min_) / 2;
  double diff_y   = center.y - (border_y_max_ + border_y_min_) / 2;
  double offset_x = diff_x / ((border_x_max_ - border_x_min_) / 2);
  double offset_y = diff_y / ((border_y_max_ - border_y_min_) / 2);
  double scale    = (border_x_max_ - border_x_min_) / 10;

  if (border_x_max_ - border_x_min_ <= 350 ||
      border_y_max_ - border_y_min_ <= 350) {
    border_x_max_ -= (1 - offset_x) * 10;
    border_y_max_ -= (1 - offset_y) * 10;
    border_x_min_ += (1 + offset_x) * 10;
    border_y_min_ += (1 + offset_y) * 10;
  } else {
    border_x_max_ -= (1 - offset_x) * scale;
    border_y_max_ -= (1 - offset_y) * scale;
    border_x_min_ += (1 + offset_x) * scale;
    border_y_min_ += (1 + offset_y) * scale;
  }

  if (border_x_max_ < border_x_min_ || border_y_max_ < border_y_min_) {
    border_x_max_ += 10;
    border_y_max_ += 10;
    border_x_min_ -= 10;
    border_y_min_ -= 10;
  }
  scale_ = width_ / (border_x_max_ - border_x_min_);
}

void WindowWrapper::ZoomOut() {
  std::lock_guard<std::mutex> lck(border_mutex_);
  cv::Point2d center = InverseTranslatePosition(mouse_position_);

  double diff_x   = center.x - (border_x_max_ + border_x_min_) / 2;
  double diff_y   = center.y - (border_y_max_ + border_y_min_) / 2;
  double offset_x = diff_x / ((border_x_max_ - border_x_min_) / 2);
  double offset_y = diff_y / ((border_y_max_ - border_y_min_) / 2);
  double scale    = (border_x_max_ - border_x_min_) / 10;
  if (border_x_max_ - border_x_min_ <= 350 ||
      border_y_max_ - border_y_min_ <= 350) {
    border_x_max_ += (1 - offset_x) * 10;
    border_y_max_ += (1 - offset_y) * 10;
    border_x_min_ -= (1 + offset_x) * 10;
    border_y_min_ -= (1 + offset_y) * 10;
  } else {
    border_x_max_ += (1 - offset_x) * scale;
    border_y_max_ += (1 - offset_y) * scale;
    border_x_min_ -= (1 + offset_x) * scale;
    border_y_min_ -= (1 + offset_y) * scale;
  }
  scale_ = width_ / (border_x_max_ - border_x_min_);
}

bool WindowWrapper::GetText() {
  std::string note =
      "(high light) input obj_p_id/lane_id and press 'Enter' use `space` "
      "replace '_'";
  std::cout
      << "output : get text doing... input obj_p_id/lane_id and press enter"
      << std::endl;
  std::string text = "";
  cv::namedWindow("get_text", cv::WINDOW_AUTOSIZE);
  cv::Mat dst = cv::Mat::zeros(cv::Size(400, 400), CV_8UC3);
  cv::Scalar text_color(0, 255, 255);
  int font_face     = cv::FONT_HERSHEY_COMPLEX;
  double font_scale = 1;
  cv::Point note_pt;
  note_pt.x = 0;
  note_pt.y = dst.rows / 4;
  dst.setTo(cv::Scalar(100, 0, 0));
  cv::putText(dst, note, note_pt, font_face, font_scale / 3, text_color);
  cv::imshow("get_text", dst);
  bool is_obj     = true;
  size_t num_size = 0;

  while (1) {
    int key = cv::waitKey(100);
    if (key == -1) {
      continue;
    }
    switch (key) {
      case 8:
        // backsapace
        if (text.size()) {
          text.back() >= 48 && text.back() <= 57 ? num_size-- : 0;
          text.pop_back();
        }
        break;
      case 13:
        // enter
        break;
      case 32:
        // space replace _
        text += '_';
        break;
      default:
        text += key;
        if (key >= 48 && key <= 57) {
          num_size++;
        }
    }
    if (key == 13) {
      break;
    }
    if (num_size == text.size()) {
      dst.setTo(cv::Scalar(100, 0, 0));
      note   = "(high light obj) input obj_p_id/lane_id and press 'Enter'";
      is_obj = true;
    } else {
      dst.setTo(cv::Scalar(80, 80, 0));
      note   = "(high light lane) input lane_id use 'space' replace '_'";
      is_obj = false;
    }
    cv::Point pt;
    pt.x = 0;
    pt.y = dst.rows / 2;
    cv::putText(dst, text, pt, font_face, font_scale, text_color);
    cv::putText(dst, note, note_pt, font_face, font_scale / 3, text_color);

    cv::imshow("get_text", dst);
  }
  std::cout << "output: " << text << std::endl;

  int obj_id = 0;
  if (is_obj) {
    for (auto ch : text) {
      obj_id *= 10;
      obj_id += (ch - '0');
    }
    AddHighLightObj(obj_id);
  } else {
    AddHighLightLaneId(text);
  }

  std::cout << "output: obj_id = " << obj_id << std::endl;
  cv::destroyWindow("get_text");
  get_text_doing = false;
  return true;
}

bool WindowWrapper::AddHighLightObj(int64_t obj_id) {
  std::lock_guard<std::mutex> high_light_lg(high_light_mutex_);
  if (obj_id) {
    high_light_obj_.insert(obj_id);
    std::cout << "output: insert obj in high " << obj_id << std::endl;
  }
  return true;
}

bool WindowWrapper::AddHighLightLaneId(std::string lane_id) {
  std::lock_guard<std::mutex> high_light_lg(high_light_mutex_);
  if (lane_id == "clear") {
    high_light_lane_id_.clear();
    return true;
  }
  if (lane_id != "") {
    high_light_lane_id_.insert(lane_id);
  } else {
    return false;
  }
  return true;
}

bool WindowWrapper::GetHighLightObj(std::unordered_set<int64_t>* objs) {
  if (!high_light_mutex_.try_lock()) {
    return false;
  }
  *objs = high_light_obj_;
  high_light_mutex_.unlock();
  return true;
}

bool WindowWrapper::GetHighLightLaneId(
    std::unordered_set<std::string>* lane_ids) {
  if (!high_light_mutex_.try_lock()) {
    return false;
  }
  *lane_ids = high_light_lane_id_;
  high_light_mutex_.unlock();
  return true;
}

bool WindowWrapper::DrawHighLightObj(
    const airos::perception::PerceptionObstacle& object) {
  GetObjectPolygon(object);
  cv::Point2f point = GetCenter();
  cv::circle(image_, point, 10, cv::Scalar(0, 255, 255), 4);
  cv::circle(image_, point, 20, cv::Scalar(0, 97, 255), 4);
  cv::circle(image_, point, 5, cv::Scalar(87, 201, 0), 4);
  return true;
}

bool WindowWrapper::DrawHighLightObj(
    const airos::perception::PerceptionObstacle& object,
    const cv::Scalar& color) {
  GetObjectPolygon(object);
  DrawPolygon(color);
  return true;
}

}  // namespace usecase
}  // namespace perception
}  // namespace airos
