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

#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

/**
 * @brief 频率计数器
 *
 */
class FrequencyCounter {
 public:
  FrequencyCounter()
      : _frequency(0)
      , _start_time(0) {}

  /**
   * @brief 触发计数，每秒计数一次
   *
   * @param frequency 输出计数值，并重置
   * @return true 有效计数
   * @return false 无效计数
   */
  bool trigger(int& frequency) {
    if (_start_time == 0) {
      _start_time = gettime_s();
    }

    _frequency++;

    int64_t cur_sys_time = gettime_s();
    if (cur_sys_time != _start_time) {
      _start_time = cur_sys_time;
      frequency   = _frequency;
      _frequency  = 0;
      return true;
    } else {
      return false;
    }
  }

 private:
  /**
   * @brief Get the time s object
   *
   * @return int64_t 返回本地时间，单位:秒
   */
  int64_t gettime_s() {
    struct timeval tv = {0, 0};
    gettimeofday(&tv, nullptr);
    return tv.tv_sec;
  }

 private:
  int _frequency;       // 计数值
  int64_t _start_time;  // 开始时间
};
