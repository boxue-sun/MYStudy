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

#include <map>
#include <string>
#include <vector>

#include <opencv/cv.hpp>
#include <yaml-cpp/yaml.h>

#include "base/common/log.h"

namespace airos {
namespace perception {
namespace algorithm {

enum ModelPrecision { FP32 = 0, FP16 = 1, INT8 = 2, TRAINNINGINT8 = 3 };
class ModelParam {
 public:
  ModelParam(
      const std::string &model_dir,
      const std::string &conffile = "config.yaml") {
    _model_dir = model_dir;
    _conf_file = conffile;
  }
  bool HasLoad() const {
    return _has_load;
  }
  bool Vaild() {
    if (!_has_load) {
      bool res = load_yaml(_model_dir + "/" + _conf_file);
      if (!res) {
        return false;
      }
    }

    if (_model_name.empty()) {
      return false;
    }
    return true;
  }
  std::string Descript();
  bool load_yaml(const std::string &file_path);

  std::string ModelDir() const {
    return _model_dir;
  }
  std::string ModelName() const {
    return _model_name;
  }
  std::string ModelFileName() const {
    return _model_filename;
  }
  std::string ParamsFileName() const {
    return _params_filename;
  }
  int DeviceID() const {
    return _device_id;
  }
  cv::Size Size() const {
    return _size;
  }
  int Channel() const {
    return _channel;
  }
  int MaxBatchSize() const {
    return _max_batch_size;
  }
  int MinSubGraphSize() const {
    return _min_subgraph_size;
  }
  ModelPrecision Precision() const {
    return _precison;
  }
  bool UseStatic() const {
    return _use_static;
  }

  std::vector<float> ConfidenceThreshold() const {
    return _types_confidences;
  }
  int DebugLevel() const {
    return _debug_level;
  }
  bool EnableTensorRT() const {
    return _enable_tensor_rt;
  }
  bool EnableMultiStream() const {
    return _enable_multi_stream;
  }

  float NmsThreshold() const {
    return _nms_threshold;
  }

  float HeightFilterRadio() const {
    return _height_filter_ratio;
  }
  float WidthFilterRadio() const {
    return _width_filter_ratio;
  }
  float MinYFilterRadio() const {
    return _minY_filter_ratio;
  }
  float HeightFilterRadioCYC() const {
    return _height_filter_ratio_cyc;
  }
  float WidthFilterRadioCYC() const {
    return _width_filter_ratio_cyc;
  }

  std::vector<int> InputShape(int level) const {
    auto it = _input_shapes.find(level);
    if (it != _input_shapes.end()) {
      return it->second;
    }
    LOG_ERROR << _model_name << " can not find input " << level << " shape";
    return std::vector<int>();
  }

  int ResizeType() const {
    return _resize_type;
  }
  int MaxGpuMallocSize() const {
    return _max_gpu_malloc_size;
  }

 private:
  std::string _model_dir;
  std::string _conf_file;
  bool _has_load = false;

  std::string _model_name;
  std::string _model_filename;
  std::string _params_filename;
  int _device_id   = 0;
  int _resize_type = 1;  //
  cv::Size _size;
  int _channel             = 0;
  int _max_batch_size      = 1;
  int _min_subgraph_size   = 3;
  ModelPrecision _precison = ModelPrecision::FP32;
  bool _use_static         = false;
  int _debug_level         = 0;
  bool _enable_tensor_rt   = true;

  std::vector<float> _types_confidences;

  // bool _model_encrypt = false;
  bool _enable_multi_stream = false;

  std::map<int, std::vector<int>> _input_shapes;

  float _nms_threshold = 0;

  float _height_filter_ratio     = 0;
  float _width_filter_ratio      = 0;
  float _minY_filter_ratio       = 0;
  float _height_filter_ratio_cyc = 0;
  float _width_filter_ratio_cyc  = 0;

  int _max_gpu_malloc_size = 0;
  bool _debug_preprocess   = false;
};

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
