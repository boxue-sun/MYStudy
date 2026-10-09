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

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#include "app/modules/v2x_application/usecase/common/common.h"

namespace airos {
namespace perception {
namespace usecase {

class Timer : public Noncopyable {
 public:
  Timer()
      : begin_(std::chrono::high_resolution_clock::now()) {}

  // reset方法
  void Reset() {
    begin_ = std::chrono::high_resolution_clock::now();
  }

  template <typename ToDuration = std::chrono::seconds>
  int64_t Duration() const {
    return std::chrono::duration_cast<ToDuration>(
               std::chrono::high_resolution_clock::now() - begin_)
        .count();
  }
  // dura_us microseconds
  int64_t DurationMicrosec() const {
    return Duration<std::chrono::microseconds>();
  }
  // dura_ms Millisecond
  int64_t DurationMillisec() const {
    return Duration<std::chrono::milliseconds>();
  }

  std::string NowStr() {
    std::stringstream ss;
    auto tm =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    ss << std::put_time(std::localtime(&tm), "%F %T");
    return ss.str();
  }

  template <typename Func, typename Duration = std::chrono::milliseconds>
  void SetTimeTask(int64_t dua, Func&& func) {
    return;
  }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> begin_;
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
