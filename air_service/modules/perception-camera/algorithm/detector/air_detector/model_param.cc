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

#include "model_param.h"

#include <map>
#include <string>
#include <vector>

#include <opencv/cv.hpp>
#include <yaml-cpp/yaml.h>

#include "base/common/log.h"

namespace airos {
namespace perception {
namespace algorithm {

bool ModelParam::load_yaml(const std::string &file_path) {
  _has_load        = true;
  int input_height = 0;
  int input_width  = 0;
  // 加载稳像参数
  bool res = true;
  try {
    YAML::Node node = YAML::LoadFile(file_path);
    if (node.IsNull()) {
      LOG_ERROR << "Load " << file_path << " failed! please check!";
      return -1;
    }
    //
    if (node["ModelName"]) {
      _model_name = node["ModelName"].as<std::string>();
    }
    if (node["ModelFileName"]) {
      _model_filename = node["ModelFileName"].as<std::string>();
    }
    if (node["ParamsFileName"]) {
      _params_filename = node["ParamsFileName"].as<std::string>();
    }
    if (node["DeviceID"]) {
      _device_id = node["DeviceID"].as<int>();
    }
    if (node["InputHeight"]) {
      input_height = node["InputHeight"].as<int>();
    }
    if (node["InputWidth"]) {
      input_width = node["InputWidth"].as<int>();
    }

    _size = cv::Size(input_width, input_height);

    if (node["InputChannel"]) {
      _channel = node["InputChannel"].as<int>();
    }

    if (node["ResizeType"]) {
      _resize_type = node["ResizeType"].as<int>();
    }

    if (node["InputShape"]) {
      for (auto it = node["InputShape"].begin(); it != node["InputShape"].end();
           ++it) {
        int level_name = -1;
        std::vector<int> shape;
        LOG_INFO << "first " << it->first.as<std::string>();
        if (it->second["Name"]) {
          level_name = it->second["Name"].as<int>();
        }
        if (it->second["Shape"]) {
          for (auto seq_it = it->second["Shape"].begin();
               seq_it != it->second["Shape"].end(); seq_it++) {
            shape.push_back(seq_it->as<int>());
          }
        }
        if (level_name >= 0 && !shape.empty()) {
          _input_shapes[level_name] = shape;
        }
      }
    }

    if (node["MaxBatchSize"]) {
      _max_batch_size = node["MaxBatchSize"].as<int>();
    }
    if (node["MinSubGraphSize"]) {
      _min_subgraph_size = node["MinSubGraphSize"].as<int>();
    }
    if (node["Precision"]) {
      std::string item_tmp = node["Precision"].as<std::string>();
      if (item_tmp == "fp32") {
        _precison = ModelPrecision::FP32;
      } else if (item_tmp == "fp16") {
        _precison = ModelPrecision::FP16;
      } else if (item_tmp == "int8") {
        _precison = ModelPrecision::INT8;
      } else {
        LOG_ERROR << "Precision not support, " << item_tmp;
      }
    }
    if (node["UseStatic"]) {
      _use_static = node["UseStatic"].as<bool>();
    }
    if (node["EnableTensorRT"]) {
      _enable_tensor_rt = node["EnableTensorRT"].as<bool>();
    }
    if (node["EnableMultiStream"]) {
      _enable_multi_stream = node["EnableMultiStream"].as<bool>();
    }

    if (node["DetectThreshold"]) {
      for (auto seq_it = node["DetectThreshold"].begin();
           seq_it != node["DetectThreshold"].end(); seq_it++) {
        _types_confidences.push_back(seq_it->as<float>());
      }
    }

    if (node["NmsThreshold"]) {
      _nms_threshold = node["NmsThreshold"].as<float>();
    }

    if (node["DebugLevel"]) {
      _debug_level = node["DebugLevel"].as<int>();
    }

    if (node["HeightFilterRadio"].IsDefined()) {
      _height_filter_ratio = node["HeightFilterRadio"].as<float>();
    }

    if (node["WidthFilterRadio"].IsDefined()) {
      _width_filter_ratio = node["WidthFilterRadio"].as<float>();
    }

    if (node["HeightFilterRadioCYC"].IsDefined()) {
      _height_filter_ratio_cyc = node["HeightFilterRadioCYC"].as<float>();
    }

    if (node["WidthFilterRadioCYC"].IsDefined()) {
      _width_filter_ratio_cyc = node["WidthFilterRadioCYC"].as<float>();
    }

    if (node["MinYFilterRadio"].IsDefined()) {
      _minY_filter_ratio = node["MinYFilterRadio"].as<float>();
    }

    if (node["MaxGpuMallocSize"].IsDefined()) {
      _max_gpu_malloc_size = node["MaxGpuMallocSize"].as<int>();
    }
  } catch (YAML::InvalidNode &in) {
    LOG_ERROR << "load yaml " << file_path
              << " with error, YAML::InvalidNode exception";
    res = false;
  } catch (YAML::TypedBadConversion<double> &bc) {
    LOG_ERROR << "load yaml " << file_path
              << " with error, YAML::TypedBadConversion exception";
    res = false;
  } catch (YAML::Exception &e) {
    LOG_ERROR << "load yaml " << file_path
              << " with error, YAML exception:" << e.what();
    res = false;
  }

  return res;
}

std::string ModelParam::Descript() {
  std::string des = "ModelName: " + _model_name + "\n";
  // des += "ModelVersion: " + _model_version + "\n";
  // des += "CreateTime: " + _create_time + "\n";
  des += "ModelFileName: " + _model_filename + "\n";
  des += "ParamsFileName: " + _params_filename + "\n";
  des += "DeviceID: " + std::to_string(_device_id) + "\n";
  des += "InputHeight: " + std::to_string(_size.height) + "\n";
  des += "InputWidth: " + std::to_string(_size.width) + "\n";
  des += "InputChannel: " + std::to_string(_channel) + "\n";
  des += "MaxBatchSize: " + std::to_string(_max_batch_size) + "\n";
  des += "EnableTensorRT: " + std::to_string(_enable_tensor_rt) + "\n";
  des += "EnableMultiStream: " + std::to_string(_enable_multi_stream) + "\n";
  des += "Precision: " + std::to_string(_precison) + "\n";
  des += "UseStatic: " + std::to_string(_use_static) + "\n";
  des += "<";
  for (unsigned i = 0; i < _types_confidences.size(); ++i) {
    des += std::to_string(_types_confidences[i]) + ", ";
  }
  des += ">\n";
  des += "HeightFilterRadio: " + std::to_string(_height_filter_ratio) + "\n";
  des += "WidthFilterRadio: " + std::to_string(_width_filter_ratio) + "\n";
  des += "HeightFilterRadioCYC: " + std::to_string(_height_filter_ratio_cyc) +
         "\n";
  des +=
      "WidthFilterRadioCYC: " + std::to_string(_width_filter_ratio_cyc) + "\n";
  des += "MinYFilterRadio: " + std::to_string(_minY_filter_ratio) + "\n";

  for (auto it = _input_shapes.begin(); it != _input_shapes.end(); it++) {
    std::string msg;
    msg += std::to_string(it->first);
    msg += ": <";
    for (auto seq_it : it->second) {
      msg += std::to_string(seq_it);
      msg += ",";
    }
    msg += ">";
    LOG_INFO << "InputShape " << msg << "\n";
    des += msg;
  }

  des += "NmsThreshold: " + std::to_string(_nms_threshold) + "\n";
  des += "Debug: " + std::to_string(_debug_level) + "\n";

  return des;
}
}  // namespace algorithm
}  // namespace perception
}  // namespace airos
