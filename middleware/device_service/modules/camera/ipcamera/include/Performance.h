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

#include <memory>
#include <string>
#include <vector>

#include "middleware/device_service/modules/camera/ipcamera/include/api-structs.h"

namespace airos {
namespace base {
namespace device {

class PrivateInfoT;

// 周期统计周期内图片总数量、bigdiff数量、最大diff、最小diff、平均diff、10分位diff、90分位diff
typedef struct stastic_data {
  stastic_data()
      : total_diff_count(0)
      , big_diff_count(0)
      , last_data(0)
      , max_diff_us(0)
      , min_diff_us(0)
      , mean_diff_us(0)
      , tenth_percentile_us(0)
      , ninetyninth_percentile_us(0)
      , last_diff_timer(0)
      , first_data(true) {}

  int total_diff_count;
  int big_diff_count;
  int64_t last_data;
  int64_t max_diff_us;
  int64_t min_diff_us;
  int64_t mean_diff_us;
  int64_t tenth_percentile_us;
  int64_t ninetyninth_percentile_us;
  int64_t last_diff_timer;
  bool first_data;
  std::vector<int64_t> vec;
} stastic_data;
class CameraPerf {
 public:
  void process(
      const std::shared_ptr<GPUImage>& img,
      const std::shared_ptr<PrivateInfoT>& info, int64_t recv_time_us,
      int64_t trick_time_us, double frame_rate);

  CameraPerf() {}

 private:
  void __statistic_recv_diff(
      const std::string& camera, const int64_t cur_time_us,
      const int64_t recv_time_us, const int64_t thresh_hold,
      const int64_t period_us = 60000000);

  void __statistic(
      const std::string& camera, const std::string& time,
      const int64_t cur_time_us, const std::string& path);

  int recvtime_reverse_count = 0;  // 时间翻转次数计数
  int recvtime_bigdiff_count = 0;

  int64_t last_timestamp_us =
      0;  // 记录上一次本类通过process接口参数recv_time_us获取的时间
  int64_t last_record_timer = 0;  // 记录上一次本类通过gettimeofday获取的时间
  bool first_time = true;

  int tricktime_reverse_count = 0;  // 绝对时间戳时间翻转次数计数
  int tricktime_bigdiff_count = 0;

  int64_t last_tricktime_us =
      0;  // 记录上一次本类通过process接口参数tricktime_us获取的时间

  int64_t recvtime_tricktime_bigdiff_count = 0;

  stastic_data recv_diff;
};

}  // namespace device
}  // namespace base
}  // namespace airos
