
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

#include "air_service/modules/perception-fusion/visualization/window_wrapper.h"

#include "absl/strings/str_cat.h"
#define CVUI_IMPLEMENTATION
#include <fstream>
#include <iostream>
// #include "apps/common/trans_tools.h"
// #include "configs/ft_config_manager.h"
#include "air_service/modules/perception-fusion/visualization/cvui.h"

namespace airos {
namespace perception {
namespace msf {

WindowWrapper::~WindowWrapper() {
  // for (auto& log_id : log_file_id_) {
  //   log_id.second.first->close();
  // }
}

void WindowWrapper::Init(int mode, double chisquare) {  // Create a window
  chisquare_       = chisquare;
  chisquare_level_ = int(20 * chisquare_);
  cvui::init("MSF");
  cv::createTrackbar("mode", "MSF", &mode_, 2);
  mode_ = mode;
  cv::setTrackbarPos("mode", "MSF", mode_);
}

void WindowWrapper::CalculateScale(
    const std::vector<msf::PerceptionObject>& objects) {
  if (!manual_) {
    std::lock_guard<std::mutex> lck(border_mutex_);
    for (int i = 0; i < objects.size(); ++i) {
      const auto& object = objects[i];

      cv::Point2d position(object.center(0), object.center(1));

      // 计算结果可视化，自动调整比例，显示全部障碍物

      border_x_min_ = position.x < border_x_min_ ? position.x : border_x_min_;
      border_x_max_ = position.x > border_x_max_ ? position.x : border_x_max_;
      border_y_min_ = position.y < border_y_min_ ? position.y : border_y_min_;
      // 为了保持横纵比例一致
      border_y_max_ = border_y_min_ + (border_x_max_ - border_x_min_);
    }
  }
  scale_ = width_ / (border_x_max_ - border_x_min_);
  // center_x_ = (border_x_max_ + border_x_min_) / 2;
  // center_y_ = (border_y_max_ + border_y_min_) / 2;
}

void WindowWrapper::CalculateScale(
    const std::vector<msf::FusionObject>& objects) {
  if (!manual_) {
    std::lock_guard<std::mutex> lck(border_mutex_);
    for (int i = 0; i < objects.size(); ++i) {
      const auto& object = objects[i];

      cv::Point2d position(object.center(0), object.center(1));

      // 计算结果可视化，自动调整比例，显示全部障碍物

      border_x_min_ = position.x < border_x_min_ ? position.x : border_x_min_;
      border_x_max_ = position.x > border_x_max_ ? position.x : border_x_max_;
      border_y_min_ = position.y < border_y_min_ ? position.y : border_y_min_;
      // 为了保持横纵比例一致
      border_y_max_ = border_y_min_ + (border_x_max_ - border_x_min_);
    }
  }
  scale_ = width_ / (border_x_max_ - border_x_min_);
  // center_x_ = (border_x_max_ + border_x_min_) / 2;
  // center_y_ = (border_y_max_ + border_y_min_) / 2;
}

void WindowWrapper::DrawInputObstacles(
    std::vector<msf::FusionInput>& fusion_input_list) {
  // for(auto &fusion_input: fusion_input_list) {
  for (int i = 0; i < fusion_input_list.size(); ++i) {
    CalculateScale(fusion_input_list[i].objects);
    int sensor_id = i + 1;
    // if(camera_switch_ != sensor_id) {
    //   return;
    // }
    for (const auto& object : fusion_input_list[i].objects) {
      std::string frame_id = object.sensor_name;

      cv::Scalar color = ChooseColor(object.type, sensor_id);

      GetObjectPolygon(object.size, object.theta, object.center);

      DrawPolygon(color);
      cv::Point2d center = GetCenter();
      if (camera_id_switch_) {
        DrawCameraId(center, sensor_id, color);
      }

      if (id_switch_) {
        DrawId(object.track_id, center, color);
      }

      if (type_switch_) {
        DrawType(object.type, center, color);
      }

      if (sub_type_switch_) {
        DrawSubType(object.sub_type, center, color);
      }
      if (error_circle_switch_) {
        DrawError(object.center_uncertainty, center, color);
      }
      if (speed_switch_) {
        DrawSpeed(object.velocity, center, color);
      }

      if (occ_state_switch_) {
        DrawOccState(object, center, color);
        DrawTruncation(object, center, color);
      }
    }
  }
  return;
}

void WindowWrapper::DrawOutputObstacles(msf::FusionOutput& fusion_output) {
  // for(auto &fusion_input: fusion_input_list) {

  // for(int i=0; i<fusion_input_list.size(); ++i) {
  CalculateScale(fusion_output.objects);
  int sensor_id = 0;
  for (const auto& object : fusion_output.objects) {
    // std::string frame_id = object.sensor_name;

    // auto& object = objects[i];
    // auto object = TransPerceptionObjectToBaseObject(perception_object);
    cv::Scalar color = ChooseColor(object.type, sensor_id);

    // GetObjectPolygon(object);
    GetObjectPolygon(object.size, object.theta, object.center);

    DrawPolygon(color);
    cv::Point2d center = GetCenter();
    // if (camera_id_switch_) {
    //   DrawCameraId(center, sensor_id, color);
    // }

    if (id_switch_) {
      DrawId(object.track_id, center, color);
    }

    if (type_switch_) {
      DrawType(object.type, center, color);
    }

    if (sub_type_switch_) {
      DrawSubType(object.sub_type, center, color);
    }
    if (error_circle_switch_) {
      DrawError(object.center_uncertainty, center, color);
    }
    if (speed_switch_) {
      DrawSpeed(object.velocity, center, color);
    }
  }

  // }
  return;
}

bool WindowWrapper::IsInPolygon(
    const std::vector<cv::Point2i>& polygon,
    cv::Point2d point) {  // can't be const reference
  if (polygon.empty()) {
    return false;
  }
  return cv::pointPolygonTest(polygon, point, false) > 0;
}

cv::Scalar WindowWrapper::ChooseColor(
    const msf::base::ObjectType& type, int sensor_id) {
  cv::Scalar color = cv::Scalar(0, 0, 0);
  if (sensor_id == 0) {
    if (type == msf::base::ObjectType::VEHICLE) {
      color = cv::Scalar(255, 0, 0);
    } else if (
        type == msf::base::ObjectType::PEDESTRIAN ||
        type == msf::base::ObjectType::BICYCLE) {
      color = cv::Scalar(0, 255, 0);
    } else {
      color = cv::Scalar(0, 0, 0);
    }
  } else {
    switch (sensor_id) {
      case -1:
        color = cv::Scalar(0, 0, 255);
        break;
      case 1:
        color = cv::Scalar(64, 0, 127);  //
        break;
      case 2:
        color = cv::Scalar(255, 255, 0);
        break;
      case 3:
        color = cv::Scalar(255, 255, 0);  //
        break;
      case 4:
        color = cv::Scalar(255, 0, 255);
        break;
      case 5:
        color = cv::Scalar(255, 0, 255);  //
        break;
      case 6:
        color = cv::Scalar(0, 64, 127);
        break;
      case 7:
        color = cv::Scalar(0, 127, 255);  //
        break;
      case 8:
        color = cv::Scalar(64, 0, 127);
        break;
      case 9:
        color = cv::Scalar(255, 0, 255);
        break;
      case 10:
        color = cv::Scalar(0, 127, 255);
        break;
      case 11:
        color = cv::Scalar(64, 0, 127);
        break;
      case 12:
        color = cv::Scalar(255, 255, 0);
        break;
      case 13:
        color = cv::Scalar(0, 0, 0);
        break;
      case 14:
        color = cv::Scalar(0, 0, 0);
        break;
      case 15:
        color = cv::Scalar(0, 0, 0);
        break;
      case 16:
        color = cv::Scalar(0, 0, 0);
        break;
      case 17:
        color = cv::Scalar(125, 125, 125);
        break;
      default:
        color = cv::Scalar(0, 0, 0);
    }
  }

  return color;
}

cv::Point2d WindowWrapper::GetCenter() {
  cv::Point2d center;
  center.x =
      (vertices_[0].x + vertices_[1].x + vertices_[2].x + vertices_[3].x) / 4;
  center.y =
      (vertices_[0].y + vertices_[1].y + vertices_[2].y + vertices_[3].y) / 4;

  return center;
}

cv::Point2d WindowWrapper::GetTopLeft() {
  cv::Point2d top_left;
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
  DrawPolygon(vertices_, color, 2, cv::LINE_AA);
}
void WindowWrapper::DrawBoldPolygon(const cv::Scalar& color) {
  DrawPolygon(vertices_, color, 8, cv::LINE_AA);
}
void WindowWrapper::DrawDashPolygon(const cv::Scalar& color) {
  DrawPolygon(vertices_, color, 1, cv::LINE_4);
}

void WindowWrapper::DrawRange(
    const cv::Point2d& point, const cv::Scalar& color) {
  cv::circle(image_, point, 0.01 * scale_, color);
  cv::circle(image_, point, 30.0 * scale_, color);
  cv::circle(image_, point, 60.0 * scale_, color);
  cv::circle(image_, point, 90.0 * scale_, color);
  cv::circle(image_, point, 120.0 * scale_, color);
}

void WindowWrapper::DrawPolygon(
    const std::vector<cv::Point2i>& vertices, const cv::Scalar& color,
    int thickness, cv::LineTypes line_type) {
  for (int i = 0; i < vertices.size(); i++) {
    cv::line(
        image_, vertices[i], vertices[(i + 1) % vertices.size()], color,
        thickness, line_type);
  }
}

void WindowWrapper::DrawObstaclesError(double x, double y, double error) {
  if (error < 0.0) {
    return;
  }
  cv::Scalar color;
  if (error < 1.50) {
    color = cv::Scalar(0, 255, 0);
  } else if (error < 2.0) {
    color = cv::Scalar(25, 255, 255);
  } else if (error < 3.5) {
    color = cv::Scalar(155, 255, 255);
  } else if (error < 10.0) {
    color = cv::Scalar(0, 0, 255);
  } else {
    color = cv::Scalar(255, 0, 0);
  }
  cv::Point2d center(x, y);
  center = TranslatePosition(center);
  cv::circle(image_, center, 0.05 * scale_, color, 2);
}

void WindowWrapper::DrawId(
    const int& track_id, const cv::Point2d& point, const cv::Scalar& color) {
  std::string id = std::to_string(track_id);
  cv::putText(
      image_, id, point, cv::FONT_HERSHEY_SIMPLEX, scale_ / kscale_id_, color,
      2);
}

void WindowWrapper::DrawYaw(
    const cv::Point2d& point1, float length, float yaw,
    const cv::Scalar& color) {
  cv::Point2d point2;
  point2.x = point1.x + length * scale_ * std::cos(yaw) * 2 / 3;
  // the y axis in reversed int image coordinates
  point2.y = point1.y - length * scale_ * std::sin(yaw) * 2 / 3;
  cv::arrowedLine(image_, point1, point2, color, 2, cv::LINE_AA);
}

void WindowWrapper::DrawSpeed(
    const msf::Point3f& velocity, const cv::Point2d& point1,
    const cv::Scalar& color) {
  cv::Point2d point2;
  point2.x = point1.x + velocity.x * scale_;
  // the y axis in reversed int image coordinates
  point2.y = point1.y - velocity.y * scale_;
  cv::Scalar speed_color;
  // if (object.sensor_type == base::SensorType::LOCALIZATION) {
  //   speed_color = cv::Scalar(0, 0, 255);
  // } else if (object.is_velocity_converged.Value()) {
  //   speed_color = cv::Scalar(0, 255, 0);
  // } else {
  //   speed_color = cv::Scalar(0, 200, 255);
  // }
  speed_color = cv::Scalar(0, 200, 255);
  // const auto& cov = object.velocity.Variance();
  // std::ostringstream var_string;
  // var_string << std::setprecision(3)
  //            << double(int((cov(0, 0) + cov(1, 1)) * 100)) / 100;
  const Eigen::Vector3d vel(velocity.x, velocity.y, velocity.z);
  std::stringstream vel_string;
  vel_string << std::setprecision(3)
             << std::sqrt(std::pow(vel[0], 2) + std::pow(vel[1], 2));
  cv::putText(
      image_, vel_string.str(), point2, cv::FONT_HERSHEY_SIMPLEX,
      scale_ / kscale_id_, cv::Scalar(0, 0, 0), 2);
  cv::line(image_, point1, point2, speed_color, 2, cv::LINE_AA);
}

void WindowWrapper::DrawOccState(
    const msf::PerceptionObject& object, const cv::Point2d& point,
    const cv::Scalar& color) {
  std::string occ_state;
  switch (object.occ_state) {
      // case base::OcclusionState::OCC_UNKNOWN:
      //   occ_state = "OCC_UNKNOWN";
      //   break;

      // case base::OcclusionState::OCC_NONE_OCCLUDED:
      //   occ_state = "OCC_NONE_OCCLUDED";
      //   break;

    case base::OcclusionState::OCC_PARTIAL_OCCLUDED:
      occ_state = "Partial";
      break;

    case base::OcclusionState::OCC_COMPLETE_OCCLUDED:
      occ_state = "Complete";
      break;

    default:
      break;
  }
  cv::putText(
      image_, occ_state, point, cv::FONT_HERSHEY_SIMPLEX, scale_ / kscale_id_,
      color, 2);
}

void WindowWrapper::DrawTruncation(
    const msf::PerceptionObject& object, const cv::Point2d& point,
    const cv::Scalar& color) {
  std::string truncation_state;
  if (object.is_truncation == true) {
    truncation_state = "truncation";
  }
  cv::putText(
      image_, truncation_state, point, cv::FONT_HERSHEY_SIMPLEX,
      scale_ / kscale_id_, color, 2);
  std::vector<cv::Point2d> vertices;
  for (auto& polygon_point : object.polygon) {
    if (polygon_point.variance.isIdentity() == true) {
      cv::Point2d point_visual(
          polygon_point.polygon.x(), polygon_point.polygon.y());
      point_visual = TranslatePosition(point_visual);
      vertices.push_back(point_visual);
    }
  }
  if (vertices.size() < 2) {
    return;
  }
  for (int i = 0; i < vertices.size() - 1; i++) {
    cv::line(
        image_, vertices[i], vertices[(i + 1)], cv::Scalar(0, 0, 255), 3.0);
  }
}

void WindowWrapper::DrawError(
    const Eigen::Matrix3d& center_var, const cv::Point2d& point,
    const cv::Scalar& color) {
  // auto sigma = object.position.Variance();
  // Eigen::EigenSolver<Eigen::Matrix3d> es(sigma);
  Eigen::Matrix2d sigma = center_var.block(0, 0, 2, 2);

  Eigen::EigenSolver<Eigen::Matrix2d> es(sigma);
  Eigen::Matrix2d d            = es.pseudoEigenvalueMatrix();
  Eigen::Matrix2d v            = es.pseudoEigenvectors();
  float dx                     = d(0, 0);
  float dy                     = d(1, 1);
  std::string eigen_values_str = absl::StrCat(dx, ", ", dy);
  // if (dx <= 0 || dy <= 0) {
  // std::cout << "covariance matrix is invalid! recieved cov is :" << sigma
  //           << std::endl;
  // return;
  // }
  float alpha = 0.0;
  if (std::fabs(sigma(0, 1)) <= std::numeric_limits<double>::epsilon() &&
      sigma(0, 0) >= sigma(1, 1)) {
    alpha = 0.0;
  } else if (
      std::fabs(sigma(0, 1)) <= std::numeric_limits<double>::epsilon() &&
      sigma(0, 0) < sigma(1, 1)) {
    alpha = M_PI_2 * to_degree_;
  } else {
    alpha = atan2(v(1, 0), v(0, 0)) * to_degree_;
  }
  alpha = (alpha < 0) ? (alpha + 360) : alpha;
  // std::cout << "alpha: " << alpha << std::endl;
  dx = 2 * sqrt(chisquare_level_ / 20.0 * std::fabs(dx)) * scale_;
  dy = 2 * sqrt(chisquare_level_ / 20.0 * std::fabs(dy)) * scale_;
  // if (dx * dy < std::numeric_limits<float>::epsilon()) {
  // }
  try {
    // coord in image and ellipse function is x to ease, y to south, so the
    // angle orientation is reversed.
    cv::ellipse(
        image_, point, cv::Size(dx / 2, dy / 2), -alpha, 0, 360, color, 2,
        cv::LINE_AA);
  } catch (const cv::Exception& e) {
    // if dx or dy is near to zero, the cv::ellipse function will crash.
    // LOG_ERROR << "Object " << object.track_id << " error circle is wrong.";
    return;
  }

  // draw eigen vector and eigen value
  if (error_circle_eigen_value_switch_) {
    cv::putText(
        image_, eigen_values_str, point, cv::FONT_HERSHEY_SIMPLEX,
        scale_ / kscale_id_, cv::Scalar(0, 0, 0), 2);
  }
}

void WindowWrapper::DrawCameraId(
    const cv::Point2d& point, int sensor_id, const cv::Scalar& color) {
  cv::putText(
      image_, std::to_string(sensor_id), point, cv::FONT_HERSHEY_SIMPLEX,
      scale_ / kscale_id_, color, 2);
}

void WindowWrapper::DrawType(
    const base::ObjectType& type, const cv::Point2d& point,
    const cv::Scalar& color) {
  std::string type_str;
  if (type == base::ObjectType::VEHICLE) {
    type_str = "Vehicle";
  } else if (type == base::ObjectType::BICYCLE) {
    type_str = "Bicycle";
  } else if (type == base::ObjectType::PEDESTRIAN) {
    type_str = "Pedestrian";
  } else if (type == base::ObjectType::UNKNOWN) {
    type_str = "Unknown";
  } else if (type == base::ObjectType::UNKNOWN_MOVABLE) {
    type_str = "Unknow movable";
  } else if (type == base::ObjectType::UNKNOWN_UNMOVABLE) {
    type_str = "Unknow Unmovable";
  }
  cv::putText(
      image_, type_str, point, cv::FONT_HERSHEY_SIMPLEX, scale_ / kscale_id_,
      color, 2);
}

void WindowWrapper::DrawSubType(
    const base::ObjectSubType& sub_type, const cv::Point2d& point,
    const cv::Scalar& color) {
  std::string sub_type_str;
  switch (sub_type) {
    case base::ObjectSubType::UNKNOWN:
      sub_type_str = "Unknown";
      break;

    case base::ObjectSubType::UNKNOWN_MOVABLE:
      sub_type_str = "Unknown_movable";
      break;

    case base::ObjectSubType::CAR:
      sub_type_str = "Car";
      break;

    case base::ObjectSubType::VAN:
      sub_type_str = "Van";
      break;

    case base::ObjectSubType::TRUCK:
      sub_type_str = "Truck";
      break;

    case base::ObjectSubType::BUS:
      sub_type_str = "Bus";
      break;

    case base::ObjectSubType::CYCLIST:
      sub_type_str = "Cyclist";
      break;

    case base::ObjectSubType::MOTORCYCLIST:
      sub_type_str = "Motorcyclist";
      break;

    case base::ObjectSubType::TRICYCLIST:
      sub_type_str = "Tricyclist";
      break;

    case base::ObjectSubType::PEDESTRIAN:
      sub_type_str = "Pedestrian";
      break;

    case base::ObjectSubType::TRAFFICCONE:
      sub_type_str = "Trafficcone";
      break;

    case base::ObjectSubType::SAFETY_TRIANGLE:
      sub_type_str = "Safety Triangle";
      break;

    default:
      break;
  }
  cv::putText(
      image_, sub_type_str, point, cv::FONT_HERSHEY_SIMPLEX,
      scale_ / kscale_id_, color, 2);
}

void WindowWrapper::GetObjectPolygon(
    const msf::SizeShape& shape, float theta, Eigen::Vector3d center) {
  vertices_.clear();
  cv::Point2d position(0, 0);
  cv::Size2f size(shape.length, shape.width);
  cv::RotatedRect rRect = cv::RotatedRect(position, size, theta * to_degree_);

  cv::Point2f vertices[4];
  rRect.points(vertices);

  // translate into image coordinates
  for (int i = 0; i < 4; ++i) {
    cv::Point2d vertex(vertices[i].x, vertices[i].y);
    vertex.x += center[0];
    vertex.y += center[1];

    vertices_.push_back(TranslatePosition(vertex));
  }

  return;
}

// from image into world coordinates
cv::Point2d WindowWrapper::InverseTranslatePosition(const cv::Point2d& point) {
  cv::Point2d trans_point;
  // trans_point.x = (point.x - border_x_min_) * scale_;
  trans_point.x = point.x / scale_ + border_x_min_;
  // trans_point.y = height_ - (point.y - border_y_min_) * scale_;
  trans_point.y = (height_ - point.y) / scale_ + border_y_min_;
  return trans_point;
}

// from world into image coordinates
cv::Point2d WindowWrapper::TranslatePosition(const cv::Point2d& point) {
  cv::Point2d trans_point;
  // trans_point.x =
  //     (point.x - border_x_min_) / (border_x_max_ - border_x_min_) * width_;
  // trans_point.y = height_ - (point.y - border_y_min_) /
  //                               (border_y_max_ - border_y_min_) * height_;
  trans_point.x = (point.x - border_x_min_) * scale_;
  trans_point.y = height_ - (point.y - border_y_min_) * scale_;
  return trans_point;
}

void WindowWrapper::CVUI() {
  MouseCallBackFunc();
  const int left_border = 5;
  int up_border         = 0;
  const int interval    = 20;
  cvui::window(image_, 0, up_border, plugin_area_, height_, "Settings");
  cvui::checkbox(
      image_, left_border, up_border += interval, "real-time result",
      &show_real_time_switch_, 0xCECECE, 0.5);
#ifdef MSF_DEBUG
  cvui::checkbox(
      image_, left_border, up_border += interval, "car result",
      &show_car_perception_switch_, 0xCECECE, 0.5);
  cvui::checkbox(
      image_, left_border, up_border += interval, "carstatus",
      &car_status_switch_, 0xCECECE, 0.5);
#endif

  cvui::checkbox(
      image_, left_border, up_border += interval, "id", &id_switch_, 0xCECECE,
      0.5);
  cvui::checkbox(
      image_, left_border, up_border += interval, "camera id",
      &camera_id_switch_, 0xCECECE, 0.5);

  cvui::checkbox(
      image_, left_border, up_border += interval, "type", &type_switch_,
      0xCECECE, 0.5);
  cvui::checkbox(
      image_, left_border, up_border += interval, "sub type", &sub_type_switch_,
      0xCECECE, 0.5);
  cvui::checkbox(
      image_, left_border, up_border += interval, "error circle",
      &error_circle_switch_, 0xCECECE, 0.5);

  cvui::checkbox(
      image_, left_border, up_border += interval, "speed", &speed_switch_,
      0xCECECE, 0.5);

  cvui::checkbox(
      image_, left_border, up_border += interval, "occ state",
      &occ_state_switch_, 0xCECECE, 0.5);
  // cv::createTrackbar("chisquare confidence", &chisquare_level_,
  //                    600);  // 0: 90%, 1: 95%, 2: 97.5%, 3: 99%

  unsigned int options =
      cvui::TRACKBAR_DISCRETE | cvui::TRACKBAR_HIDE_SEGMENT_LABELS;

  // cvui::trackbar(image_, 0, up_border += interval, plugin_area_, //TODO
  // sliding roller to control which camera is displayed
  //                &camera_switch_, (int)0, (int)20, 20, "%.0Lf", options,
  //                (int)1);

  // int width = dashboard_->MaxSize() > 0 ? dashboard_->MaxSize() : 1;
  // cvui::trackbar(image_, 0, up_border += 2 * interval, plugin_area_,
  //                &dashboard_switch_, (int)0, width, width, "%.0Lf", options,
  //                (int)1);

  cvui::update();
}

void WindowWrapper::Show() {
  // show the image
  // cv::putText(image_, std::to_string(border_x_min_), cv::Point2d(5, 50),
  //             cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
  // cv::putText(image_, std::to_string(border_y_min_), cv::Point2d(5, 70),
  //             cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
  // cv::putText(image_, std::to_string(border_x_max_), cv::Point2d(5, 90),
  //             cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
  // cv::putText(image_, std::to_string(border_y_max_), cv::Point2d(5, 110),
  //             cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);

  // if (inputting_id_) {
  //   cv::putText(image_, std::string("id: "), cv::Point2d(width_ - 100, 20),
  //               cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
  //   cv::putText(image_, std::to_string(single_obj_.Id()),
  //               cv::Point2d(width_ - 75, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5,
  //               cv::Scalar(0, 0, 0), 2);
  // }
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

  CVUI();
  cv::imshow("MSF", image_);
}

void WindowWrapper::AddRuler(cv::Point2i point, bool head) {
  if (head) {
    ruler_head_ = point;
  } else {
    ruler_tail_ = point;
  }
}

void WindowWrapper::RecordLogPolygon(bool reset) {
  if (reset) {
    logging_ = false;
    log_polygon_.clear();
    return;
  }
  static cv::Point2i last_pixel(0, 0);
  if (log_polygon_.empty()) {
    last_pixel.x = 0;
    last_pixel.y = 0;
  }
  if (std::hypot(
          mouse_position_.x - last_pixel.x, mouse_position_.y - last_pixel.y) >
      5) {
    last_pixel = mouse_position_;
    log_polygon_.push_back(mouse_position_);
  }
}

void WindowWrapper::DrawLogPolygon() {
  for (int i = 0; i < log_polygon_.size(); i++) {
    cv::line(
        image_, log_polygon_[i], log_polygon_[(i + 1) % log_polygon_.size()],
        cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
  }
}

void WindowWrapper::MouseCallBackFunc() {
  mouse_position_         = cvui::mouse("MSF");
  static int drag_start_x = 0;
  static int drag_start_y = 0;
  if (!log_swtich_) {
    RecordLogPolygon([] {
      bool reset = true;
      return reset;
    });
  } else {
    DrawLogPolygon();
  }
  if (cvui::mouse(cvui::LEFT_BUTTON, cvui::DOWN) &&
      mouse_position_.x > plugin_area_) {
    drag_start_x = mouse_position_.x;
    drag_start_y = mouse_position_.y;
    if (log_swtich_) {
      RecordLogPolygon([] {
        bool reset = true;
        return reset;
      });
    }
  } /*  else if (cvui::mouse(cvui::LEFT_BUTTON, cvui::UP)) {
   } */
  else if (
      cvui::mouse(cvui::LEFT_BUTTON, cvui::IS_DOWN) &&
      mouse_position_.x > plugin_area_) {
    int diff_x = mouse_position_.x - drag_start_x;
    int diff_y = mouse_position_.y - drag_start_y;
    if (std::abs(diff_x) > 1 || std::abs(diff_y) > 1) {
      SetManual();
    }
    if (log_swtich_) {
      RecordLogPolygon();
    } else {
      Drag(diff_x, diff_y);
    }

    drag_start_x = mouse_position_.x;
    drag_start_y = mouse_position_.y;
  } else if (
      cvui::mouse(cvui::LEFT_BUTTON, cvui::UP) &&
      mouse_position_.x > plugin_area_) {
    if (log_swtich_) {
      logging_ = true;
    }
  }

  if (cvui::mouse(cvui::RIGHT_BUTTON, cvui::DOWN)) {
    AddRuler(mouse_position_, true);
    AddRuler(mouse_position_, false);
  } else if (cvui::mouse(cvui::RIGHT_BUTTON, cvui::IS_DOWN)) {
    // add ruler tools
    AddRuler(mouse_position_, false);
  } else if (cvui::mouse(cvui::RIGHT_BUTTON, cvui::UP)) {
    double dis = cv::norm(
        InverseTranslatePosition(ruler_tail_) -
        InverseTranslatePosition(ruler_head_));
    if (dis > 0.1) {
      cv::Point2d point = InverseTranslatePosition(ruler_head_);
      std::cout << std::setprecision(16) << "head: [" << point.x << ", "
                << point.y << "], ";
      point = InverseTranslatePosition(ruler_tail_);
      std::cout << std::setprecision(16) << "tail: [" << point.x << ","
                << point.y << "], ";
      std::cout << "distance: " << dis << std::endl;
    }
    AddRuler(cv::Point2i(0, 0), true);
    AddRuler(cv::Point2i(0, 0), false);
  }

  if (cvui::mouse(cvui::MIDDLE_WHEEL, cvui::FORWARD)) {
    SetManual();
    ZoomIn();
  } else if (cvui::mouse(cvui::MIDDLE_WHEEL, cvui::BACKWARD)) {
    SetManual();
    ZoomOut();
  }

  if (cvui::mouse(cvui::RIGHT_BUTTON, cvui::CLICK)) {
    rclick_.x = mouse_position_.x;
    rclick_.y = mouse_position_.y;
  }
}

void WindowWrapper::KeyCallbackFunc(int key) {
  switch (key) {
    case 'y':
      manual_ = false;
      RestBorder();
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
      break;
    // case 'i':
    //   if (!inputting_id_) {
    //     single_obj_.SetId(0);
    //     inputting_id_ = true;
    //   } else {
    //     inputting_id_ = false;
    //     single_obj_.Init();
    //   }
    //   break;
    // case 'p':
    //   single_obj_.SetPredicted(!single_obj_.IsPredicted());
    case 'r':
      cv::imwrite("./result.jpg", image_);
  }
  // if (inputting_id_ && key >= '0' && key <= '9') {
  //   CatId(key);
  // }
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
  std::lock_guard<std::mutex> lck(border_mutex_);
  border_x_max_ -= x / scale_;
  border_y_max_ += y / scale_;
  border_x_min_ -= x / scale_;
  border_y_min_ += y / scale_;
}

void WindowWrapper::DragUp() {
  std::lock_guard<std::mutex> lck(border_mutex_);
  // border_x_max_ += 10;
  border_y_max_ += 10;
  // border_x_min_ += 10;
  border_y_min_ += 10;
}

void WindowWrapper::DragDown() {
  std::lock_guard<std::mutex> lck(border_mutex_);
  // border_x_max_ += 10;
  border_y_max_ -= 10;
  // border_x_min_ += 10;
  border_y_min_ -= 10;
}

void WindowWrapper::DragLeft() {
  std::lock_guard<std::mutex> lck(border_mutex_);
  border_x_max_ -= 10;
  // border_y_max_ += 10;
  border_x_min_ -= 10;
  // border_y_min_ += 10;
}

void WindowWrapper::DragRight() {
  std::lock_guard<std::mutex> lck(border_mutex_);
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
  double diff_x      = center.x - (border_x_max_ + border_x_min_) / 2;
  double diff_y      = center.y - (border_y_max_ + border_y_min_) / 2;
  double offset_x    = diff_x / ((border_x_max_ - border_x_min_) / 2);
  double offset_y    = diff_y / ((border_y_max_ - border_y_min_) / 2);
  double scale       = (border_x_max_ - border_x_min_) / 10;

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
}

void WindowWrapper::ZoomOut() {
  std::lock_guard<std::mutex> lck(border_mutex_);
  cv::Point2d center = InverseTranslatePosition(mouse_position_);
  double diff_x      = center.x - (border_x_max_ + border_x_min_) / 2;
  double diff_y      = center.y - (border_y_max_ + border_y_min_) / 2;
  double offset_x    = diff_x / ((border_x_max_ - border_x_min_) / 2);
  double offset_y    = diff_y / ((border_y_max_ - border_y_min_) / 2);
  double scale       = (border_x_max_ - border_x_min_) / 10;
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
}

// void WindowWrapper::CatId(int key) {
//   int num = static_cast<int>(key - '0');
//   cv::putText(image_, std::to_string(single_obj_.CatId(num)),
//               cv::Point2d(15, 140), cv::FONT_HERSHEY_SIMPLEX, 0.5,
//               cv::Scalar(0, 0, 0), 2);
// }

void WindowWrapper::RestBorder() {
  std::lock_guard<std::mutex> lck(border_mutex_);
  border_x_min_ = 1e9;
  border_y_min_ = 1e9;
  border_x_max_ = 0;
  border_y_max_ = 0;
}

}  // namespace msf
}  // namespace perception
}  // namespace airos
