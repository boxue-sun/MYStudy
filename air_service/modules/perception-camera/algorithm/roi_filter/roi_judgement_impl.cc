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

#include "air_service/modules/perception-camera/algorithm/roi_filter/roi_judgement_impl.h"

#include <math.h>

#include <iostream>

#include "base/common/log.h"

namespace airos {
namespace perception {
namespace algorithm {

bool ROIJudgement::Init(const InitParam& init_param) {
  mask_file_name_ = init_param.mask_file;
  if (mask_file_name_.empty()) {
    LOG_WARN << "mask_file_name_ is empty, not use filter";
    return false;
  }
  auto mask_img = cv::imread(mask_file_name_, cv::IMREAD_GRAYSCALE);
  if (mask_img.empty()) {
    LOG_ERROR << "mask_img filename is " << mask_file_name_
              << ", cannot read cv::Mat";
    return false;
  }
  integral_mask_img_.reset(new cv::Mat());
  cv::threshold(mask_img, mask_img, 128, 255, CV_THRESH_BINARY);
  cv::integral(mask_img, *integral_mask_img_, CV_32S);

  image_width_  = mask_img.cols;
  image_height_ = mask_img.rows;
  LOG_INFO << "image_width is " << image_width_ << ",image_height is "
           << image_height_;
  overlap_rate_ = init_param.overlap_rate;
  return true;
}

int ROIJudgement::Process(const Box2f& box, ObjectType type, int& result) {
  result          = 1;
  float bbox_xmin = box.left_top.x;
  float bbox_ymin = box.left_top.y;
  float bbox_xmax = box.right_bottom.x;
  float bbox_ymax = box.right_bottom.y;

  // 对超出图像边界的框做截断处理
  float xmin = (bbox_xmin < 0) ? 0 : bbox_xmin;
  float xmax = (bbox_xmax > image_width_ - 1) ? image_width_ - 1 : bbox_xmax;
  float ymin = (bbox_ymin < 0) ? 0 : bbox_ymin;
  float ymax = (bbox_ymax > image_height_ - 1) ? image_height_ - 1 : bbox_ymax;
  int integral_val = integral_mask_img_->at<int>(ymax + 1, xmax + 1) -
                     integral_mask_img_->at<int>(ymax + 1, xmin) -
                     integral_mask_img_->at<int>(ymin, xmax + 1) +
                     integral_mask_img_->at<int>(ymin, xmin);
  if ((integral_val < (xmax - xmin) * (ymax - ymin) * overlap_rate_ * 255) ||
      (((ymax - ymin) < 50 && ymax > 1000 &&
        type == airos::perception::algorithm::ObjectType::PEDESTRIAN) ||
       ((ymax - ymin) < 50 && (xmax - xmin) < 90 && ymax > 1000 &&
        type == airos::perception::algorithm::ObjectType::CAR))) {
    // VLOG(49) << "ROI Filter track id " << (*iter)->_track_id << ", obstacle
    // "; VLOG(49) << "no ROI Filter track id" << ", obstacle: "<< type ;
    result = 0;
  }
  return 0;
}

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
