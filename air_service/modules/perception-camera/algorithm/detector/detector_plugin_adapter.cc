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

#include "air_service/modules/perception-camera/algorithm/detector/detector_plugin_adapter.h"

#include <string>

#include "base/common/log.h"
#include "base/common/time_util.h"
#include "base/io/protobuf_util.h"

namespace airos {
namespace perception {
namespace camera {

inline algorithm::BaseObjectDetecter::InitParam GetAlgorithmParam(
    const DetectorParam& detector_param) {
  algorithm::BaseObjectDetecter::InitParam ret;
  ret.id                = detector_param.id();
  ret.model_dir         = detector_param.model_dir();
  ret.gpu_id            = detector_param.gpu_id();
  ret.mask_img_filename = "";  // NOT USE
  return ret;
}

bool DetectorPluginAdapter::ReadConfFile(const std::string& conf_file) {
  if (airos::base::ParseProtobufFromFile<DetectorParam>(
          conf_file, &detector_param_) == false)
    return false;
  return true;
}

bool DetectorPluginAdapter::Init(const airos::base::PluginParam& param) {
  if (ReadConfFile(param.config_file) == false) return false;
  p_detector_.reset(algorithm::BaseObjectDetecterRegisterer::GetInstanceByName(
      detector_param_.detector_name()));
  if (p_detector_ == nullptr) return false;
  return p_detector_->Init(GetAlgorithmParam(detector_param_));
}

bool DetectorPluginAdapter::Run(PerceptionFrame& data) {
  double start_time = airos::base::TimeUtil::GetCurrentTime();
  auto& image       = data.image;
  int ret           = p_detector_->Process(
      image->device_data(), image->channels(), image->cols(), image->rows(),
      data.detect_ret);
  if (ret != 0) {
    LOG_ERROR << "detect fail";
  }
  double finish_time = airos::base::TimeUtil::GetCurrentTime();
  LOG_INFO << "perception detect ret count  " << data.detect_ret.size();
  LOG_INFO << "perception detect cost "
           << static_cast<int>((finish_time - start_time) * 1000) << " ms";
  return ret == 0 ? true : false;
}

}  // namespace camera
}  // namespace perception
}  // namespace airos
