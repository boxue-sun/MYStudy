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
#include <unordered_map>
#include <vector>

#include "app/modules/v2x_application/usecase/base/base_alg.h"
#include "app/modules/v2x_application/usecase/base/frame_data.h"
#include "app/modules/v2x_application/usecase/common/common.h"
#include "app/modules/v2x_application/usecase/common/semaphore.h"
#include "app/modules/v2x_application/usecase/common/timer.h"

namespace airos {
namespace perception {
namespace usecase {

class Runner : public Noncopyable {
 public:
  using AlgorithmAndParamsMap = std::unordered_map<
      std::shared_ptr<BaseAlgorithmModule>, std::shared_ptr<BaseParams>>;
  using StringVectorPtr = std::shared_ptr<std::vector<std::string>>;
  using FrameDataPtr    = std::shared_ptr<FrameData>;

  void Init(bool rebuild_data = true);
  std::shared_ptr<airos::usecase::EventOutputResult> Run(
      const std::shared_ptr<const airos::perception::PerceptionObstacles>&
          obstacles);
  std::shared_ptr<airos::usecase::EventOutputResult> Run(
      const FrameDataPtr& data,
      const std::shared_ptr<const airos::perception::PerceptionObstacles>&
          obstacles);
  FrameDataPtr& GetFrameData() {
    return data_;
  }
  void ClearEvents();
  void SetPipeline(const StringVectorPtr& pipeline) {
    pipelines_ = pipeline;
  }

 private:
  std::shared_ptr<airos::usecase::EventOutputResult> RunFrameData(
      const std::shared_ptr<const airos::perception::PerceptionObstacles>&
          obstacles);

 private:
  // std::vector<AlgorithmAndParamsPair> algos_params_pair_;
  AlgorithmAndParamsMap map_;
  StringVectorPtr pipelines_ = nullptr;
  FrameDataPtr data_         = nullptr;
  std::shared_ptr<Semaphore> end_semp_ =
      nullptr;  // 等待并行的多个算法线程结束，再进行数据后处理和传输
  Timer timer_;
};

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
