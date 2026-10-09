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

#include <atomic>
#include <memory>
#include <mutex>

#include "app/modules/v2x_application/usecase/base/config_manager.h"
#include "app/modules/v2x_application/usecase/base/frame_data.h"
#include "app/modules/v2x_application/usecase/common/common.h"
#include "app/modules/v2x_application/usecase/common/factory.hpp"
#include "app/modules/v2x_application/usecase/common/semaphore.h"
#include "app/modules/v2x_application/usecase/common/timer.h"

namespace airos {
namespace perception {
namespace usecase {

class BaseAlgorithmModule : public Noncopyable {
 public:
  virtual ~BaseAlgorithmModule()                             = default;
  virtual bool Init(const std::shared_ptr<BaseParams>& conf) = 0;
  virtual void Proc(const std::shared_ptr<FrameData>& data)  = 0;
  virtual void Mock() {}

  virtual void Run(const std::shared_ptr<FrameData>& data = nullptr) {
    timer_.Reset();
    if (is_mock_) {
      Mock();
    }
    ProcFrameData(data);
    APP_LOG_INFO << "debug-> " << conf_->Name()
             << " cost: " << timer_.DurationMicrosec();
  }
  virtual bool Load(const std::shared_ptr<BaseParams>& conf) {
    return InitParams(conf);
  }

  virtual void SetSemaphore(const std::shared_ptr<Semaphore>& end_semp) {}
  virtual bool IsAsync() const {
    return is_async_;
  }

 protected:
  virtual bool InitParams(const std::shared_ptr<BaseParams>& conf) {
    conf_    = conf;
    is_mock_ = conf_->GetVal("mock").Cast<bool>();
    return Init(conf);
  }

  virtual void ProcFrameData(const std::shared_ptr<FrameData>& data) {
    if (data) {
      Proc(data);
    }
  }

 protected:
  bool is_mock_  = false;
  bool is_async_ = false;
  std::shared_ptr<BaseParams> conf_;
  static std::atomic<int64_t> global_id_;
  Timer timer_;
};

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
