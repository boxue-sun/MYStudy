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

#include <string>

#include "air_service/modules/perception-camera/algorithm/interface/algorithm_base.h"
#include "base/plugin/registerer.h"

namespace airos {
namespace perception {
namespace algorithm {

/**
 * @brief
 *  对超出图像边界的框做截断处理
 *  以下情况result 为1：
 *
 *  fisheye: 与roi 区域重叠大于等于overlap_rate
 *  非fisheye:
 *      1.  与roi 区域重叠超过overlap_rate
 *      2. 对于ObjectType为PEDESTRIAN，除了1之外，还要求height >= 50，或者 ymax
 * <= 1000
 *      3. 对于ObjectType为CAR，，除了1之外，还要求height >= 50，或者width >=
 * 90，或者ymax <= 1000
 *
 */

class BaseROIJudgement {
 public:
  struct InitParam {
    std::string mask_file;
    double overlap_rate = 0.5;
  };

 public:
  virtual bool Init(const InitParam& init_param) = 0;

  virtual int Process(const Box2f& box, ObjectType type, int& result) = 0;

  virtual std::string Name() const = 0;
};

PERCEPTION_REGISTER_REGISTERER(BaseROIJudgement);
#define REGISTER_ROI_JUDGEMENT(name) \
  PERCEPTION_REGISTER_CLASS(BaseROIJudgement, name)

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
