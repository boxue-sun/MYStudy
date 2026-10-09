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

#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <paddle_inference_api.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "base/common/log.h"
#include "gpu.h"
#include "model_param.h"

#define UNUSED_VARIABLE(x) ((void)(x))

namespace airos {
namespace perception {
namespace algorithm {

class ModelProcessOption {
 public:
  explicit ModelProcessOption(const std::string &model_dir)
      : _model_dir(model_dir) {}

  std::string _model_dir;
  std::string _model_config_name = "config.yaml";
  int _thread_api_num            = 1;
  int _gpu_id                    = -1;
};
template <typename T>
class ModelResult {
 public:
  int _level = -1;
  std::vector<T> _data;
  std::vector<int> _shape;
  std::string Shape() const {
    std::string info = std::to_string(_shape.size());
    info += " : ";
    for (const auto it : _shape) {
      info += ", " + std::to_string(it);
    }
    return info;
  }
};

class ModelProcess {
 public:
  explicit ModelProcess(const ModelProcessOption &opt)
      : _param(opt._model_dir, opt._model_config_name)
      , _gpu_id(opt._gpu_id) {}
  ~ModelProcess() {}

  std::string ModelName() const {
    return _param.ModelName();
  }
  bool Init();
  ModelParam Params() {
    return _param;
  }

  bool SetGpuID(int gpu_id) {
    if (_predictor) {
      LOG_FATAL << "invalid set gpu id, has init";
    }
    _gpu_id = gpu_id;
    return true;
  }
  int UseGpuID() const {
    return _gpu_id;
  }

  template <typename T>
  bool SetInputGpuData(const unsigned level, const T *data, unsigned int len) {
    if (_level_shape_len[level] != len) {
      LOG_ERROR << "gpu shape set len " << _level_shape_len[level]
                << ", in num " << len;
      return false;
    }
    float *input_data =
        _input_ts[level]->mutable_data<float>(paddle::PaddlePlace::kGPU);
    cudaMemcpy(input_data, data, len * sizeof(float), cudaMemcpyDeviceToDevice);

    return true;
  }

  bool Process() {
    bool ok = _predictor->ZeroCopyRun();
    if (!ok) {
      LOG_ERROR << "predictor->ZeroCopyRun return false";
      return {};
    }
    return ok;
  }
  template <typename T>
  bool SetInputData(const unsigned level, const T *data, unsigned int len) {
    if (_level_shape_len[level] != len) {
      LOG_ERROR << "level " << level << " gpu shape set len "
                << _level_shape_len[level] << ", in num " << len;
      return false;
    }
    _input_ts[level]->copy_from_cpu(data);

    return true;
  }
  template <class T>
  ModelResult<T> GetOutputData(const unsigned level) {
    auto output_names = _predictor->GetOutputNames();
    if (level >= output_names.size()) {
      LOG_ERROR << "expect output level " << level << ", but real level max "
                << output_names.size() - 1;
      return ModelResult<T>();
    }
    // out level prob
    ModelResult<T> out_data;
    std::vector<int> output_shape = _output_ts[level]->shape();

    std::vector<std::vector<size_t>> lod = _output_ts[level]->lod();
    if (lod.size() > 0) {
      for (size_t j = 1; j < lod[0].size(); j++) {
        out_data._shape.push_back(lod[0][j] - lod[0][j - 1]);
      }
    }

    int out_num = 1;
    for (auto it : output_shape) {
      out_num *= it;
    }

    out_data._data.resize(out_num);
    _output_ts[level]->copy_to_cpu(out_data._data.data());
    out_data._level = level;
    return out_data;
  }

 private:
  bool prepareTRTConfig(paddle::AnalysisConfig *config);
  bool set_moel_data(paddle::AnalysisConfig *config);

  ModelParam _param;
  int _gpu_id                    = -1;
  unsigned int _input_level_num  = 1;
  unsigned int _output_level_num = 1;

  std::vector<unsigned int> _level_shape_len;

  std::unique_ptr<paddle::PaddlePredictor> _predictor;

  std::vector<std::unique_ptr<paddle::ZeroCopyTensor>> _input_ts;
  std::vector<std::unique_ptr<paddle::ZeroCopyTensor>> _output_ts;

  bool _debug = true;
};

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
