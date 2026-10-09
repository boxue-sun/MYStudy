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

#include "air_service/modules/perception-camera/algorithm/roi_filter/roi_judgement_adapter.h"

#include <string>
#include <vector>

#include "base/common/log.h"
#include "base/io/protobuf_util.h"

namespace airos {
namespace perception {
namespace camera {

inline algorithm::BaseROIJudgement::InitParam GetAlgorithmParam(
    const RoiFilterParam& roi_param) {
  algorithm::BaseROIJudgement::InitParam ret;
  ret.mask_file    = roi_param.mask_file_path();
  ret.overlap_rate = roi_param.overlap_rate();
  return ret;
}

bool RoiPluginAdapter::ReadConfFile(const std::string& conf_file) {
  if (airos::base::ParseProtobufFromFile<RoiFilterParam>(
          conf_file, &roi_param_) == false)
    return false;
  return true;
}

bool RoiPluginAdapter::Init(const airos::base::PluginParam& param) {
  if (ReadConfFile(param.config_file) == false) {
    LOG_ERROR << "RoiPluginAdapter ReadConfFile fail, config file is"
              << param.config_file;
    return false;
  }
  p_roi_filter_.reset(algorithm::BaseROIJudgementRegisterer::GetInstanceByName(
      roi_param_.roi_filter_name()));
  if (p_roi_filter_ == nullptr) {
    LOG_ERROR << "RoiPluginAdapter creat instance fail, roi name is"
              << roi_param_.roi_filter_name();
    return false;
  }
  return p_roi_filter_->Init(GetAlgorithmParam(roi_param_));
}

bool RoiPluginAdapter::Run(PerceptionFrame& data) {
  if (p_roi_filter_ == nullptr) return false;
  std::vector<algorithm::ObjectDetectInfoPtr>& objects = data.detect_ret;
  for (auto iter = objects.begin(); iter < objects.end();) {
    int ret;
    p_roi_filter_->Process((*iter)->box, (*iter)->type, ret);
    if (ret == 0)
      iter = objects.erase(iter);
    else
      iter++;
  }
  return true;
}

}  // namespace camera
}  // namespace perception
}  // namespace airos
