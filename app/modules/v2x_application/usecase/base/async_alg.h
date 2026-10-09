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
#include <thread>

#include "app/modules/v2x_application/usecase/base/base_alg.h"

namespace airos {
namespace perception {
namespace usecase {

class AsyncAlgorithmModule : public BaseAlgorithmModule {
 public:
  AsyncAlgorithmModule()
      : is_async_(true) {}
  virtual bool Init(const std::shared_ptr<BaseParams>& conf) = 0;
  virtual void Proc(const std::shared_ptr<FrameData>& data)  = 0;

  virtual void Mock() {}
  virtual bool IsAsync() const {
    return is_async_;
  }

  void Run(const std::shared_ptr<FrameData>& data = nullptr) override {
    ProcFrameData(data);  // 得到数据，通知线程执行
  }

  bool Load(const std::shared_ptr<BaseParams>& conf) override {
    return InitParams(conf);  // 初始化拉起执行算法的线程
  }

  void SetSemaphore(const std::shared_ptr<Semaphore>& end_semp) override {
    end_semp_ = end_semp;
  }

  virtual ~AsyncAlgorithmModule() {
    if (run_thread_->joinable()) {
      run_thread_->join();
    }
  }

 protected:
  void ProcFrameData(const std::shared_ptr<FrameData>& data) override {
    data_ = data;
    start_semp_.StartNotify();
  }

  bool InitParams(const std::shared_ptr<BaseParams>& conf) override {
    conf_    = conf;
    is_mock_ = conf_->GetVal("mock").Cast<bool>();
    bool res = Init(conf);
    start_semp_.SetWorkerNum(1);
    run_thread_.reset(new std::thread(
        &AsyncAlgorithmModule::RunThread,
        this));  // 执行算法的线程
    return res;
  }

  void RunThread() {
    while (true) {
      APP_LOG_INFO << "debug-> run_thread before wait " << conf_->Name();
      start_semp_.WaitStart();
      APP_LOG_INFO << "debug-> " << conf_->Name() << " in";
      timer_.Reset();
      if (is_mock_) {
        Mock();
      }
      if (data_) {
        Proc(data_);
      }
      APP_LOG_INFO << "debug-> " << conf_->Name()
               << " cost: " << timer_.DurationMicrosec();
      // need not to lock
      data_ = nullptr;
      start_semp_.SetDataCount();
      end_semp_->EndNotify();
    }
  }

 protected:
  Semaphore start_semp_;
  std::shared_ptr<Semaphore> end_semp_;
  bool is_async_ = true;
  std::shared_ptr<FrameData> data_;
  std::unique_ptr<std::thread> run_thread_;
};

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
