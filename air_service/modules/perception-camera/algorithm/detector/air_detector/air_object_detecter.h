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

#include <iostream>
#include <memory>
#include <numeric>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "air_service/modules/perception-camera/algorithm/interface/object_detecter.h"
#include "base/blob/cuda_util.h"
#include "data_type.h"
#include "model_process.h"

namespace airos {
namespace perception {
namespace algorithm {

#define math_PI 3.14159265

using airos::perception::algorithm::ObjectDetectInfoPtr;
using DetectInitParam =
    airos::perception::algorithm::BaseObjectDetecter::InitParam;

class AirObjectDetecter
    : public airos::perception::algorithm::BaseObjectDetecter {
 public:
  using ExecFunc = std::function<void()>;

  // for compatible old api;
  AirObjectDetecter(const AirObjectDetecter &) = delete;

  AirObjectDetecter() {}
  bool Init(const InitParam &init_param) override;

  ~AirObjectDetecter() {
    LOG_INFO << "AirObjectDetecter destructor ";
    if (_gpu_pre_process_data) {
      airos::base::CudaUtil::free(_gpu_pre_process_data);
      _gpu_pre_process_data = nullptr;
    }
    if (_gpu_img_data) {
      airos::base::CudaUtil::free(_gpu_img_data);
      _gpu_img_data = nullptr;
    }
  }

  std::string ModelName() {
    return _proc->ModelName();
  }
  std::string ID() const override {
    return _model_options.id;
  }

  int Process(
      const unsigned char *gpu_data, int channel, int height, int width,
      std::vector<ObjectDetectInfoPtr> &output) override;

 private:
  int process(const GpuImg &gpu_img, std::vector<ObjectDetectInfoPtr> &output);
  bool pre_process(
      const GpuImg &img, const std::vector<int> &shape, int input_level);
  int pre_process_gpu(
      const uint8_t *gpu_img, int channel, int width, int height, int step);
  int post_process(
      std::vector<ObjectDetectInfoPtr> &output_data, int height, int width);
  void inter_class_nms(
      std::vector<ObjectDetectInfoPtr> &output, float nms_thresh);

 private:
  InitParam _model_options;

  std::shared_ptr<ModelProcess> _proc;
  int _height_width = 0;

  std::vector<float> _threshold;
  float _nms_thresh = 0.6;

  std::vector<float> bin_center_ = {
      math_PI / 6.0, math_PI / 2.0, math_PI * 5.0 / 6.0};

  int _debug_level               = 0;
  float _height_filter_ratio     = 0;
  float _width_filter_ratio      = 0;
  float _height_filter_ratio_cyc = 0;
  float _width_filter_ratio_cyc  = 0;
  float _minY_filter_ratio       = 0;
  bool _enable_filter            = false;

  unsigned char *_gpu_img_data = nullptr;
  int _max_img_size            = 1920 * 1080 * 3;
  float *_gpu_pre_process_data = nullptr;
  std::vector<ObjectDetectInfoPtr> _frame_all_objs;
};

REGISTER_OBSTACLE_DETECTOR(AirObjectDetecter);

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
