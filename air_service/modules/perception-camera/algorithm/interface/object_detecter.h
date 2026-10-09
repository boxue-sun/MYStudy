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

#include "air_service/modules/perception-camera/algorithm/interface/algorithm_base.h"
#include "air_service/modules/perception-camera/algorithm/interface/object_detect_info.h"
#include "base/plugin/registerer.h"

namespace airos {
namespace perception {
namespace algorithm {

class BaseObjectDetecter {
 public:
  struct InitParam {
    std::string id;         // 透传字段，id标识接口
    std::string model_dir;  // 模型路径
    int gpu_id = -1;
    std::string mask_img_filename;  // 检测感兴趣区域;黑白图, 白色感兴趣区域,
                                    // 黑色非感兴趣区域; 不设置则分析整张图
  };

 public:
  virtual ~BaseObjectDetecter()                  = default;
  virtual std::string ID() const                 = 0;
  virtual bool Init(const InitParam &init_param) = 0;
  // 0 success; otherwise fail
  // virtual int Process(const cv::Mat &in_data,
  // std::vector<ObjectDetectInfoPtr> &output) = 0;
  virtual int Process(
      const unsigned char *gpu_data, int channel, int height, int width,
      std::vector<ObjectDetectInfoPtr> &output) = 0;
};

PERCEPTION_REGISTER_REGISTERER(BaseObjectDetecter);
#define REGISTER_OBSTACLE_DETECTOR(name) \
  PERCEPTION_REGISTER_CLASS(BaseObjectDetecter, name)

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
