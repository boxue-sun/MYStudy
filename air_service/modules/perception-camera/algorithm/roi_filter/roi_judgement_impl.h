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

#include <opencv2/opencv.hpp>

#include "air_service/modules/perception-camera/algorithm/interface/roi_judgement.h"

namespace airos {
namespace perception {
namespace algorithm {

class ROIJudgement : public BaseROIJudgement {
 public:
  ROIJudgement() = default;

  ~ROIJudgement() = default;

  bool Init(const InitParam& init_param) override;

  int Process(const Box2f& box, ObjectType type, int& result) override;

  std::string Name() const override {
    return module_name_;
  };

 private:
  const std::string module_name_ = "ROIJudgement";
  std::string mask_file_name_;
  double overlap_rate_ = 0.5;
  std::unique_ptr<cv::Mat> integral_mask_img_;
  int image_width_  = 0;
  int image_height_ = 0;
};

REGISTER_ROI_JUDGEMENT(ROIJudgement)
}  // namespace algorithm
}  // namespace perception
}  // namespace airos
