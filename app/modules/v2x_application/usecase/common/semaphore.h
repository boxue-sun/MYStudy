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
#include <condition_variable>
#include <memory>
#include <mutex>

namespace airos {
namespace perception {
namespace usecase {

class Semaphore {
 public:
  Semaphore()
      : thread_cnt_(0)
      , data_count_(0) {}

  void StartNotify() {
    std::unique_lock<std::mutex> lock(m_);
    data_count_++;
    cv_.notify_all();
  }

  void WaitStart() {
    std::unique_lock<std::mutex> lock(m_);
    if (work_num_ == 0) {
      return;
    }
    if (!is_done_) {
      cv_.wait(lock, [this]() {
        if (data_count_) {
          APP_LOG_INFO << "debug-> cv_.wait true";
          return true;
        }
        APP_LOG_INFO << "debug-> cv_.wait false";
        return false;
      });
    }
    is_done_ = false;
  }

  void WaitEnd() {
    std::unique_lock<std::mutex> lock(m_);
    if (work_num_ == 0) {
      return;
    }
    if (!is_done_) {
      cv_.wait(lock);
    }
    is_done_ = false;
  }

  void EndNotify() {
    std::unique_lock<std::mutex> lock(m_);
    ++thread_cnt_;
    if (thread_cnt_ == work_num_) {
      cv_.notify_all();
      thread_cnt_ = 0;
      is_done_    = true;
    }
  }

  void SetDataCount() {
    data_count_ = 0;
  }

  void SetWorkerNum(int num) {
    std::unique_lock<std::mutex> lock(m_);
    work_num_ = num;
  }

 private:
  std::mutex m_;
  std::condition_variable cv_;
  int thread_cnt_;        // 线程计数
  bool is_done_ = false;  // 所有线程是否工作完成
  int work_num_ = 1;      // 工作线程数量

  std::atomic<unsigned int> data_count_;
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
