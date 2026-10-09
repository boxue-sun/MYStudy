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

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "air_service/framework/perception-fusion/proto/perception_fusion_config.pb.h"

#include "air_service/framework/perception-fusion/interface/base_perception_fusion.h"
#include "base/plugin/algorithm_loader/dynamic_modules.h"
#include "middleware/runtime/src/air_middleware_component.h"

#ifdef MSF_VIZ
#include "air_service/modules/perception-fusion/proto/fusion_params.pb.h"
#include "air_service/modules/perception-fusion/proto/multi_sensor_fusion_config.pb.h"

#include "air_service/modules/perception-fusion/visualization/visualization_internal.h"
#endif

namespace airos {
namespace perception {
namespace fusion {
using airos::perception::PerceptionObstacles;

#define SENSOR_FUSION_COMPONENT AIROS_COMPONENT_CLASS_NAME(MsfComponent)
class SENSOR_FUSION_COMPONENT : public middleware::ComponentAdapter<> {
 public:
  SENSOR_FUSION_COMPONENT() {}
  SENSOR_FUSION_COMPONENT(const SENSOR_FUSION_COMPONENT &) = delete;

  SENSOR_FUSION_COMPONENT &operator=(const SENSOR_FUSION_COMPONENT &) = delete;

  ~SENSOR_FUSION_COMPONENT() override;
  bool Init() override;

 private:
  bool InitConfig();
  bool InitFusion();
  bool InitNode();
  // bool InitInputList();
  bool InitVisualization();
  void InternalCallback(
      int sequence,
      const std::shared_ptr<const PerceptionObstacles> &callback_msg);
  void MainLoop();
  void MsfProc(std::vector<std::shared_ptr<const PerceptionObstacles>>
                   &fusion_input_list);
  // void SerializeMsg(const msf::FusionOutput &message,
  //                   std::shared_ptr<PerceptionObstacles> output_msg,
  //                   int error_code);
  bool CheckTimestamp(std::shared_ptr<PerceptionObstacles> output_msg);

 private:
  std::unique_ptr<BasePerceptionFusion> air_fusion_;
  std::vector<std::shared_ptr<const PerceptionObstacles>> fusion_input_list_;
  PerceptionFusionParam fusion_base_conf_;

  std::unique_ptr<std::thread> main_loop_;
  std::mutex loop_mutex_;
  std::condition_variable new_msg_;

  std::shared_ptr<middleware::AirMiddlewareWriter<PerceptionObstacles>>
      perception_writer_;

  std::vector<
      std::shared_ptr<middleware::AirMiddlewareReader<PerceptionObstacles>>>
      reader_list_;

  int seq_num_      = 0;
  bool initialized_ = false;
  bool is_exit_     = false;

#ifdef MSF_VIZ
  std::shared_ptr<msf::MSFVisualKernal> msf_visual_kernal_;
  airos::perception::msf::MsfConfig air_msf_config_;
#endif
};

REGISTER_AIROS_COMPONENT_NULLTYPE_CLASS(MsfComponent);

}  // namespace fusion
}  // namespace perception
}  // namespace airos
