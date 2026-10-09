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

#include "middleware/device_service/modules/camera/ipcamera/include/Performance.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <numeric>

#include "base/common/log.h"

#define OUTPUT_TIME_INTERVAL (1800 * 1000000)  // (30 * 60 * 1000 * 1000)
#define FIRST_TIME_ 1.5 * 60 * 1000 * 1000
#define TIMESTAMP_BIG_COUNT_LIMIT 30

namespace airos {
namespace base {
namespace device {

void CameraPerf::__statistic(
    const std::string& camera, const std::string& time,
    const int64_t cur_time_us, const std::string& path) {
  stastic_data& data    = recv_diff;
  data.total_diff_count = data.vec.size();
  if (0 == data.total_diff_count) {
    return;
  }

  std::sort(data.vec.begin(), data.vec.end());  // 默认升序
  data.tenth_percentile_us       = data.vec[data.total_diff_count / 10];
  data.ninetyninth_percentile_us = data.vec[data.total_diff_count / 100 * 99];
  data.min_diff_us               = data.vec[0];
  data.max_diff_us               = data.vec[data.total_diff_count - 1];
  data.mean_diff_us = std::accumulate(data.vec.begin(), data.vec.end(), 0) /
                      data.total_diff_count;

  std::ofstream log_stream;
  log_stream.open(path, std::ofstream::app);
  if (log_stream.is_open()) {
    log_stream << time << " <STATISTIC>" << camera << "time:" << cur_time_us
               << ","
               << "total_cnt:" << data.total_diff_count << ","
               << "big_diff_cnt:" << data.big_diff_count << ","
               << "min_diff_us:" << data.min_diff_us << ","
               << "max_diff_us:" << data.max_diff_us << ","
               << "tenth_percentile_us:" << data.tenth_percentile_us << ","
               << "ninetyninth_percentile_us:" << data.ninetyninth_percentile_us
               << ","
               << "mean_diff_us:" << data.mean_diff_us << "\n";

    log_stream.flush();
    log_stream.close();
  } else {
    LOG_WARN << "<STATISTIC>:open " << path << "failed!";
  }
}

void CameraPerf::__statistic_recv_diff(
    const std::string& camera, const int64_t cur_time_us,
    const int64_t recv_time_us, const int64_t thresh_hold,
    const int64_t period_us) {
  if (recv_diff.first_data) {
    recv_diff.big_diff_count  = 0;
    recv_diff.last_diff_timer = cur_time_us;
    recv_diff.last_data       = recv_time_us;
    recv_diff.first_data      = false;
    LOG_WARN << "<STATISTIC>:statistic start time is " << cur_time_us;
    return;
  }

  int64_t recv_diff_us = recv_time_us - recv_diff.last_data;
  // 理论上recv diff都是大于0的，异常数据不处理
  if (recv_diff_us > 0) {
    recv_diff.vec.push_back(recv_diff_us);
  }
  if (recv_diff_us > thresh_hold) {
    ++recv_diff.big_diff_count;
    LOG_WARN << "<STATISTIC>:bigdiff " << recv_time_us << "-"
             << recv_diff.last_data << " = " << recv_diff_us;
  }

  if (cur_time_us - recv_diff.last_diff_timer >= period_us) {
    struct tm tm;
    time_t ts = time(0);
    localtime_r(&ts, &tm);
    char buf[128];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    std::string cur_time(buf);
    std::string path = "/xlog/log/recv_diff_statistic_" + camera + "-" +
                       cur_time.substr(0, 10) + ".log";
    __statistic(camera, cur_time, cur_time_us, path);
    recv_diff.last_diff_timer = cur_time_us;
    recv_diff.big_diff_count  = 0;
    recv_diff.vec.clear();
  }

  recv_diff.last_data = recv_time_us;
}

void CameraPerf::process(
    const std::shared_ptr<GPUImage>& img,
    const std::shared_ptr<PrivateInfoT>& info, int64_t recv_time_us,
    int64_t trick_time_us, double frame_rate) {
  // 接口内常量
  static const int TIMESTAMP_MIX_INTERVAL =
      0;  // 时间戳最小间隔，小于 TIMESTAMP_MIX_INTERVAL us,则认为时间翻转
  static const int TIMESTAMP_MAX_INTERVAL =
      66000;  // 时间戳最大间隔，大于 TIMESTAMP_MAX_INTERVAL
              // us,则认为时间间隔过大

  // 记录当前时间
  timeval now;
  gettimeofday(&now, NULL);
  // 记录当前通过gettimeofday获取到的时间，用于和 last_record_timer 对比
  int64_t cur_record_timer = now.tv_sec * 1000000 + now.tv_usec;

  // 统计recv_diff
  __statistic_recv_diff(
      img->channel_str, cur_record_timer, recv_time_us, TIMESTAMP_MAX_INTERVAL);

  // 判断是否为第1次调用本接口，第一次时，更新last_timestamp、last_record_timer及first_time
  if (first_time) {
    last_timestamp_us = recv_time_us;
    last_tricktime_us = trick_time_us;
    last_record_timer = cur_record_timer;
    first_time        = false;
    return;
  }

  // 记录当前通过process接口参数recv_time_us获取到的时间,用于和last_timestamp比对
  int64_t cur_timestamp_us = recv_time_us;

  // 记录当前通过process接口参数abs_time_us获取到的时间，用于和last_abs_timestamp比对
  int64_t cur_tricktime_us = trick_time_us;

  // 时间异常判断
  if (cur_timestamp_us - last_timestamp_us <= TIMESTAMP_MIX_INTERVAL) {
    recvtime_reverse_count++;
  } else if (cur_timestamp_us - last_timestamp_us > TIMESTAMP_MAX_INTERVAL) {
    recvtime_bigdiff_count++;
  }

  // trick时间戳异常判断
  if (cur_tricktime_us - last_tricktime_us < TIMESTAMP_MIX_INTERVAL) {
    tricktime_reverse_count++;
    LOG_WARN << "<DEBUG_TIME_PERF>" << img->channel_str
             << " reverse cur_tricktime_us:" << cur_tricktime_us
             << ",last_tricktime_us:" << last_tricktime_us;
  } else if (cur_tricktime_us - last_tricktime_us == TIMESTAMP_MIX_INTERVAL) {
    tricktime_reverse_count++;
    LOG_WARN << "<DEBUG_TIME_PERF>" << img->channel_str
             << " equal cur_tricktime_us:" << cur_tricktime_us
             << ",last_tricktime_us:" << last_tricktime_us;
  } else if (cur_tricktime_us - last_tricktime_us > TIMESTAMP_MAX_INTERVAL) {
    tricktime_bigdiff_count++;
    LOG_WARN << "<DEBUG_TIME_PERF>" << img->channel_str
             << " bigdiff cur_tricktime_us:" << cur_tricktime_us
             << ",last_tricktime_us:" << last_tricktime_us;
  }

  // 如果 recvtime和 tricktime相差 5s， 则记录
  if (cur_timestamp_us - cur_tricktime_us > 5000000 ||
      cur_tricktime_us - cur_timestamp_us > 5000000) {
    recvtime_tricktime_bigdiff_count++;
    LOG_WARN << "<DEBUG_TIME_PERF>" << img->channel_str
             << ",cur_record_timer:" << cur_record_timer
             << ",cur_recvtime_us:" << cur_timestamp_us
             << ",cur_tricktime_us:" << cur_tricktime_us;
  }

  // 监控结果输出判断, 每隔OUTPUT_TIME_INTERVAL  us 输出一次
  if (cur_record_timer - last_record_timer >= OUTPUT_TIME_INTERVAL) {
    // driver 日志记录
    LOG_WARN << "<TIME_PERF>" << img->channel_str
             << " last 30 min timestamp reverse count "
             << recvtime_reverse_count << ", timestamp big diff count "
             << recvtime_bigdiff_count << " Packet loss rate "
             << (recvtime_bigdiff_count / 30.0 / 60 / 15)
             << ", tricktime reverse count " << tricktime_reverse_count
             << ", tricktime big diff count " << tricktime_bigdiff_count
             << ", recvtime and tricktime diff count "
             << recvtime_tricktime_bigdiff_count;
    // for log file
    struct tm tm;
    time_t ts = time(0);
    localtime_r(&ts, &tm);
    char buf[128];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    std::string cur_time(buf);
    std::ofstream big_diff_log;
    std::string root_dir = "/xlog/log/";
    big_diff_log.open(
        root_dir + "monitor_big_diff_" + img->channel_str + "-" +
            cur_time.substr(0, 10) + ".log",
        std::ofstream::app);
    big_diff_log << cur_time << " <TIME_PERF>" << img->channel_str
                 << " last 30 min timestamp reverse count "
                 << recvtime_reverse_count << ", timestamp big diff count "
                 << recvtime_bigdiff_count << " Packet loss rate "
                 << (recvtime_bigdiff_count / 30.0 / 60 / 15)
                 << ", tricktime reverse count " << tricktime_reverse_count
                 << ", tricktime big diff count " << tricktime_bigdiff_count
                 << ", recvtime and tricktime diff count "
                 << recvtime_tricktime_bigdiff_count << "\n";
    big_diff_log.flush();
    big_diff_log.close();

    // 更新 last_record_timer；用于下一次监控结果输出判断
    last_record_timer = cur_record_timer;

    // 重置recvtime_reverse_count，recvtime_bigdiff_count；用于下一次监控间隔的结果记录
    recvtime_reverse_count = 0;
    recvtime_bigdiff_count = 0;

    // reset tricktime_reverse_count, tricktime_bigdiff_count
    tricktime_reverse_count = 0;
    tricktime_bigdiff_count = 0;

    recvtime_tricktime_bigdiff_count = 0;
  }

  // 更新 last_timestamp
  last_timestamp_us = cur_timestamp_us;

  // update last_tricktime_us
  last_tricktime_us = cur_tricktime_us;
}

}  // namespace device
}  // namespace base
}  // namespace airos
