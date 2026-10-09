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

#include "air_service/modules/perception-fusion/visualization/visualization_internal.h"

#include <iostream>

#include "visualization_trans_tool.h"

namespace airos {
namespace perception {
namespace msf {

int MSFVisualKernal::Init(int mode, double chisquare) {
  mode_      = mode;
  chisquare_ = chisquare;
  window_wrapper_.Init(mode, chisquare);
  disp_thread_.reset(
      new std::thread(std::bind(&MSFVisualKernal::DispThread, this)));
  return 1;
}

void MSFVisualKernal::SetMode(int mode) {
  mode_ = mode;
}

bool MSFVisualKernal::UpdateObstacles(
    const std::vector<std::shared_ptr<
        const airos::perception::PerceptionObstacles>>& fusion_input_list,
    const std::shared_ptr<const airos::perception::PerceptionObstacles>&
        fusion_output,
    MsfConfig& fusion_param) {
  {
    if (fusion_input_list.size() != fusion_param.input_size()) return false;
    std::vector<msf::FusionInput> sdk_input(fusion_input_list.size());
    for (int i = 0; i < fusion_input_list.size(); ++i) {
      if (fusion_input_list[i] != nullptr)
        visualization::tools::TransTools::PbsToObjects(
            *(fusion_input_list[i]), &sdk_input[i]);
      sdk_input[i].index = i;
      sdk_input[i].collision_line_fusion =
          fusion_param.input(i).collision_line_fusion();
    }
    msf::FusionOutput sdk_output;
    if (visualization::tools::TransTools::PbsToObjectsOutput(
            fusion_output, &sdk_output) == false) {
      return false;
    }
    std::lock_guard<std::mutex> lock(data_mutex_);
    fusion_output_     = sdk_output;
    fusion_input_list_ = sdk_input;
  }
  return true;
}

void MSFVisualKernal::DispThread() {
  /**
   * @brief the main loop
   *
   */
  bool init = false;
  while (!is_exit_) {
    if (init == false) {
      window_wrapper_.ResetBackground();
      window_wrapper_.Show();
      window_wrapper_.KeyCallbackFunc(cv::waitKey(1));
      init = true;
      continue;
    }
    int mode = window_wrapper_.Mode();
    if (mode != mode_) {
      SetMode(mode);
    }
    window_wrapper_.ResetBackground();
    bool show_real_time = window_wrapper_.ShowRealTimeResult();

    bool show_new   = false;
    bool show_input = false;
    if (show_real_time) {
      switch (mode_) {
        case 0:
          show_input = true;
          break;
        case 1:
          show_new = true;
          break;
        case 2:
          show_new   = true;
          show_input = true;
          break;
        default:
          break;
      }
    }

    if (show_new) {
      std::lock_guard<std::mutex> lock(data_mutex_);
      window_wrapper_.DrawOutputObstacles(fusion_output_);
    }

    if (show_input) {
      std::lock_guard<std::mutex> lock(data_mutex_);
      window_wrapper_.DrawInputObstacles(fusion_input_list_);
    }

    window_wrapper_.Show();
    window_wrapper_.KeyCallbackFunc(cv::waitKey(1));
  }
}

}  // namespace msf
}  // namespace perception
}  // namespace airos
