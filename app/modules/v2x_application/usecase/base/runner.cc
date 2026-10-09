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

#include "app/modules/v2x_application/usecase/base/runner.h"

#include <utility>

#include "app/modules/v2x_application/usecase/base/config_manager.h"
#include "base/common/singleton.h"

namespace airos {
namespace perception {
namespace usecase {

void Runner::Init(bool rebuild_data) {
  end_semp_.reset(new Semaphore());
  end_semp_->SetWorkerNum(0);
  if (rebuild_data) {
    data_.reset(new FrameTrackData());
    data_->Init();
    base::Singleton<EventCollector>::get_instance()->Init();
  }
  if (!pipelines_) {
    return;
  }

  auto config_manager = base::Singleton<ConfigManager>::get_instance();

  int num_worker = 0;
  for (const auto& pipeline : *pipelines_) {
    auto param = Factory<BaseParams>::Instance().GetShared(pipeline);
    if (!param) {
      continue;
    }
    if (!config_manager->LoadFor(param)) {
      continue;
    }
    bool active = false;
    try {
      active = param->GetVal("active").Cast<bool>();
    } catch (...) {
      continue;
    }
    if (!active) {
      continue;
    }
    auto algorithm = Factory<BaseAlgorithmModule>::Instance().GetShared(
        pipeline);  // 从注册过的工厂中生成指定算法对象
    if (!algorithm) {
      APP_LOG_WARN << "debug-> " << pipeline << " build failed";
      continue;
    }
    APP_LOG_WARN << "debug-> " << pipeline << " has joined pipeline";
    algorithm->IsAsync() ? ++num_worker : 0;
    map_.insert(std::make_pair(algorithm, param));
  }

  end_semp_->SetWorkerNum(num_worker);
  for (auto& item : map_) {
    if (item.first->IsAsync()) {
      item.first->SetSemaphore(end_semp_);
    }
    if (!item.first->Load(item.second)) {
      APP_LOG_ERROR << "debug-> " << item.second->Name() << " init Failed!!!";
    }
  }
}

std::shared_ptr<airos::usecase::EventOutputResult> Runner::Run(
    const std::shared_ptr<const airos::perception::PerceptionObstacles>&
        obstacles) {
  timer_.Reset();
  data_->Update(obstacles);  // 障碍物匹配
  APP_LOG_INFO << "debug-> update frame cost: " << timer_.DurationMicrosec();
  return RunFrameData(obstacles);
}

std::shared_ptr<airos::usecase::EventOutputResult> Runner::Run(
    const FrameDataPtr& data,
    const std::shared_ptr<const airos::perception::PerceptionObstacles>&
        obstacles) {
  data_ = data;
  return RunFrameData(obstacles);
}

std::shared_ptr<airos::usecase::EventOutputResult> Runner::RunFrameData(
    const std::shared_ptr<const airos::perception::PerceptionObstacles>&
        obstacles) {
  for (auto& item : map_) {
    item.first->Run(data_);
  }
  end_semp_->WaitEnd();  // 等待所有线程结束
  return data_->Dump(obstacles);
}

void Runner::ClearEvents() {
  base::Singleton<EventCollector>::get_instance()->Clear();
}

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
