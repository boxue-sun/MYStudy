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

#include <sys/time.h>

#include <cmath>

#include "base/common/log.h"

namespace airos {
namespace perception {
namespace algorithm {

class Latency {
 public:
  Latency() {
    gettimeofday(&_timep_begin, nullptr);
  }
  void Start() {
    gettimeofday(&_timep_begin, nullptr);
  }
  int64 duration() {
    gettimeofday(&_timep_stop, nullptr);  // 该函数在sys/time.h头文件中
    return diff(&_timep_begin, &_timep_stop);
  }

 private:
  inline int64 diff(struct timeval *t1, struct timeval *t2) {
    int64 d =
        (t1->tv_sec - t2->tv_sec) * 1000000LL + (t1->tv_usec - t2->tv_usec);
    return std::abs(d);
  }
  struct timeval _timep_begin;
  struct timeval _timep_stop;
};

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
