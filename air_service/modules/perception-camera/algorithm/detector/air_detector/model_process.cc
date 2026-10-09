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

#include "model_process.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <opencv2/opencv.hpp>

#include "latency.h"

namespace airos {
namespace perception {
namespace algorithm {

bool ModelProcess::prepareTRTConfig(paddle::AnalysisConfig *config) {
  set_moel_data(config);
  config->EnableUseGpu(100, _gpu_id);
  config->SwitchUseFeedFetchOps(false);
  config->SwitchSpecifyInputNames(true);
  if (_param.EnableMultiStream()) {
    config->EnableGpuMultiStream();
    LOG_INFO << "Enable GpuMultiStream";
  }

  if (_param.EnableTensorRT()) {
    bool use_static = _param.UseStatic();
    LOG_INFO << _param.ModelName() << " use static " << use_static;
    if (_param.Precision() == ModelPrecision::FP32) {
      config->EnableTensorRtEngine(
          1 << 30, _param.MaxBatchSize(), _param.MinSubGraphSize(),
          paddle::AnalysisConfig::Precision::kFloat32, use_static, false);
      LOG_INFO << _param.ModelName() << "enable Precision Fp32, MaxBatchSize "
               << _param.MaxBatchSize();
    } else if (_param.Precision() == ModelPrecision::FP16) {
      config->EnableTensorRtEngine(
          1 << 30, _param.MaxBatchSize(), _param.MinSubGraphSize(),
          paddle::AnalysisConfig::Precision::kHalf, use_static, false);
      LOG_INFO << _param.ModelName() << "enable Precision Fp16, MaxBatchSize "
               << _param.MaxBatchSize();
    } else if (_param.Precision() == ModelPrecision::INT8) {
      config->EnableTensorRtEngine(
          1 << 30, _param.MaxBatchSize(), _param.MinSubGraphSize(),
          paddle::AnalysisConfig::Precision::kInt8, use_static, true);
      LOG_INFO << _param.ModelName() << " enable Precision int8, MaxBatchSize "
               << _param.MaxBatchSize();
    }
  }

  return true;
}

bool ModelProcess::Init() {
  if (!_param.Vaild()) {
    LOG_ERROR << "para valid fail";
    return false;
  }
  LOG_INFO << _param.Descript();

  if (_gpu_id < 0) {
    _gpu_id = _param.DeviceID();
  }
  LOG_INFO << "gpu id = " << _gpu_id;

  LOG_INFO << _param.ModelName() << " use paddle version "
           << paddle::get_version();

  // DeviceEnable(_gpu_id);//TODO need nvcc

  paddle::AnalysisConfig config;
  prepareTRTConfig(&config);

  LOG_INFO << _param.ModelName() << " run PaddlePredictor";
  _predictor       = std::move(CreatePaddlePredictor(config));
  auto input_names = _predictor->GetInputNames();
  _input_level_num = input_names.size();
  for (size_t i = 0; i < input_names.size(); i++) {
    _input_ts.emplace_back(
        std::move(_predictor->GetInputTensor(input_names[i])));
  }
  _level_shape_len.resize(_input_level_num);
  for (size_t i = 0; i < _input_ts.size(); i++) {
    std::vector<int> shape = _param.InputShape(i);
    if (shape.empty()) {
      LOG_FATAL << "input shape is empty";
    }
    unsigned shape_len = 1;
    for (size_t i = 0; i < shape.size(); i++) {
      shape_len *= shape[i];
    }
    _level_shape_len[i] = shape_len;
    _input_ts[i]->Reshape(shape);
  }

  auto output_names = _predictor->GetOutputNames();
  for (size_t i = 0; i < output_names.size(); i++) {
    _output_ts.emplace_back(
        std::move(_predictor->GetOutputTensor(output_names[i])));
  }

  LOG_INFO << "CreatePaddlePredictor finish, mkldnn = "
           << config.mkldnn_quantizer_enabled();
  return true;
}

bool ModelProcess::set_moel_data(paddle::AnalysisConfig *config) {
  std::string model_file = _param.ModelDir() + "/__model__";
  std::string param_file = _param.ModelDir() + "/__params__";
  if (!_param.ModelFileName().empty()) {
    model_file = _param.ModelDir() + "/" + _param.ModelFileName();
  }
  if (!_param.ParamsFileName().empty()) {
    param_file = _param.ModelDir() + "/" + _param.ParamsFileName();
  }
  LOG_INFO << "model filename " << model_file;
  LOG_INFO << "params filename " << param_file;

  LOG_INFO << _param.ModelName() << " load model from file";
  if (!_param.ParamsFileName().empty() && _param.ParamsFileName() == "None") {
    LOG_INFO << _param.ModelName() << " Only Model File";
    config->SetModel(_param.ModelDir());
  } else {
    config->SetModel(model_file, param_file);
  }

  return true;
}

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
