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

#include "gat_monitor.h"

#include <iostream>

#include "base/common/log.h"
#include "gat_util.h"

namespace os {
namespace v2x {
namespace device {

GatMonitor::GatMonitor() {}

GatMonitor::~GatMonitor() {
  if (timer_fd_color_state_ != nullptr) {
    timer_delete(timer_fd_color_state_);
  }

  if (timer_fd_curr_plan_step_ != nullptr) {
    timer_delete(timer_fd_curr_plan_step_);
  }
}

bool GatMonitor::Init() {
  if (!InitTimerColorState()) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "init timer light state failed.";
    return false;
  }
  if (!InitTimerCurrPlanStep()) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "init timer current period failed.";
    return false;
  }
  return true;
}

void GatMonitor::IncRecvFrameCounter() {
  recv_frame_counter_++;
}

void GatMonitor::IncGatPacketCounter() {
  gat_packet_counter_++;
  UpdateTimestampGatPacket();
}

void GatMonitor::IncGatBadPacketCounter() {
  gat_bad_packet_counter_++;

  TRAFFIC_LIFGT_SERVICE_LOG_WARN << "bad packet=" << gat_bad_packet_counter_
           << ", total packet=" << gat_packet_counter_
           << ", recv frame=" << recv_frame_counter_;
}

void GatMonitor::IncColorStatePacketCounter() {
  color_state_packet_counter_++;
  UpdateTimestampColorState();
}

void GatMonitor::IncCurrPlanStepPacketCounter() {
  curr_plan_step_packet_counter_++;
  UpdateTimestampCurrPlanStep();
}

float GatMonitor::GetColorStatePacketFreq() {
  float freq =
      float(color_state_packet_counter_) / float(kColorStateSamplingSec);
  return freq;
}

void GatMonitor::ZeroColorStatePacketCounter() {
  color_state_packet_counter_ = 0;
}

float GatMonitor::GetCurrPlanStepPacketFreq() {
  float freq =
      float(curr_plan_step_packet_counter_) / float(kCurrPlanStepSamplingSec);
  return freq;
}

void GatMonitor::ZeroCurrPlanStepPacketCounter() {
  curr_plan_step_packet_counter_ = 0;
}

void GatMonitor::TimeoutColorState(__sigval_t arg) {
  GatMonitor *monitor_ptr = reinterpret_cast<GatMonitor *>(arg.sival_ptr);
  float freq              = monitor_ptr->GetColorStatePacketFreq();
  if (freq <= 8.0) {
    std::string msg = "Light State Frequency too low, Current Frequency = " +
                      std::to_string(freq);
    TRAFFIC_LIFGT_SERVICE_LOG_WARN << msg;
  } else if (freq > 10.0) {
    std::string msg = "Light State Frequency too high, Current Frequency = " +
                      std::to_string(freq);
    TRAFFIC_LIFGT_SERVICE_LOG_WARN << msg;
  }
  monitor_ptr->ZeroColorStatePacketCounter();
}

void GatMonitor::TimeoutCurrPlanStep(__sigval_t arg) {
  GatMonitor *monitor_ptr = reinterpret_cast<GatMonitor *>(arg.sival_ptr);
  float freq              = monitor_ptr->GetCurrPlanStepPacketFreq();
  if (freq <= 8.0) {
    std::string msg = "Curr Period Frequency too low, Current Frequency = " +
                      std::to_string(freq);
    TRAFFIC_LIFGT_SERVICE_LOG_WARN << msg;
  } else if (freq > 10.0) {
    std::string msg = "Curr Period Frequency too high, Current Frequency = " +
                      std::to_string(freq);
    TRAFFIC_LIFGT_SERVICE_LOG_WARN << msg;
  }
  monitor_ptr->ZeroCurrPlanStepPacketCounter();
}

bool GatMonitor::IsDataExpired() {
  if ((GatUtil::GetCurTimestampMsec() - timestamp_color_state_) >
      kColorStateExpireMS) {
    return true;
  }
  return false;
}

bool GatMonitor::IsRemoteAlive() {
  if ((GatUtil::GetCurTimestampMsec() - timestamp_gat_packet_) <=
      kGatPacketExpireMS) {
    return true;
  } else {
    return false;
  }
}

void GatMonitor::FlushColorState(
    std::map<uint8_t, GatLightState> &color_state) {
  IncColorStatePacketCounter();

  CheckColorState(color_state);

  std::unique_lock<std::mutex> guard(color_state_mutex_);
  color_state_.clear();
  color_state_ = color_state;
}

void GatMonitor::FlushCurrPlanStep(
    std::map<uint8_t, std::vector<GatLightStep>> &curr_plan_step) {
  IncCurrPlanStepPacketCounter();

  CheckCurrPlanStep(curr_plan_step);

  std::unique_lock<std::mutex> guard(curr_plan_step_mutex_);
  curr_plan_step_.clear();
  curr_plan_step_ = curr_plan_step;
}

void GatMonitor::CheckColorState(
    const std::map<uint8_t, GatLightState> &color_state) {
  std::map<uint8_t, GatLightState> cur_lights_state;
  {
    std::unique_lock<std::mutex> guard(color_state_mutex_);
    cur_lights_state = color_state_;
  }
  std::map<uint8_t, std::vector<GatLightStep>> cur_curr_period;
  {
    std::unique_lock<std::mutex> guard(curr_plan_step_mutex_);
    cur_curr_period = curr_plan_step_;
  }

  // 检查灯色状态中的黑灯
  for (auto &kv : color_state) {
    if (kv.second.color == 0) {
      std::string msg = "light color invalid, light_id=" +
                        std::to_string(unsigned(kv.first)) + " color is black.";
      TRAFFIC_LIFGT_SERVICE_LOG_WARN << "" << msg;
    }
  }

  if (!cur_curr_period.empty()) {
    for (auto &kv : color_state) {
      // 检查灯色状态中的灯组号和方案色步信息中灯组号的一致性
      if (cur_curr_period.count(kv.first) < 1) {
        std::string msg = "light color id not matched, light_id=" +
                          std::to_string(unsigned(kv.first));
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "" << msg;
      }

      // 检查灯色状态中灯类型和放射色步信息中灯类型的一致性
      if (cur_curr_period.count(kv.first) > 0) {
        if ((cur_curr_period[kv.first].size() > 0) &&
            (cur_curr_period[kv.first][0].type != kv.second.type)) {
          std::string msg =
              "light type not matched, light_id=" +
              std::to_string(unsigned(kv.first)) +
              ", LightState color=" + std::to_string(unsigned(kv.second.type)) +
              +", CurrPeriod color=" +
              std::to_string(unsigned(cur_curr_period[kv.first][0].type));
          TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "" << msg;
        }
      }
    }
  }

  // 检查灯色状态中的倒计时异常
  if (!cur_lights_state.empty()) {
    for (auto &kv : color_state) {
      const GatLightState &one_new_state = kv.second;
      GatLightState &one_old_state       = cur_lights_state[kv.first];

      // 灯类型改变异常
      if (one_new_state.type != one_old_state.type) {
        std::string msg =
            "light type changed invalid light_id=" +
            std::to_string(unsigned(kv.first)) +
            ", prev type=" + std::to_string(unsigned(one_old_state.type)) +
            ", curr type=" + std::to_string(unsigned(one_new_state.type));
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "" << msg;
      }

      // 灯色改变异常（倒计时未到1）
      if ((one_new_state.color != one_old_state.color) &&
          (one_old_state.countdown != 1)) {
        std::string msg =
            "light countdown invalid when color change, light_id=" +
            std::to_string(unsigned(kv.first)) + ", countdown is 0.";
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "" << msg;
      }

      // 倒计时异常（值为非法0）
      if (one_new_state.countdown == 0) {
        std::string msg =
            "light countdown invalid when color change, light_id=" +
            std::to_string(unsigned(kv.first)) + ", prev(countdown=" +
            std::to_string(unsigned(one_old_state.countdown)) +
            ",color=" + std::to_string(unsigned(one_old_state.color)) +
            "), now(countdown=" +
            std::to_string(unsigned(one_new_state.countdown)) +
            ",color=" + std::to_string(unsigned(one_new_state.color));
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "" << msg;
      }

      // 倒计时异常（超时不变）
      uint64_t time_ms_delta =
          one_new_state.timestamp - one_old_state.timestamp;
      if (time_ms_delta > kColorStateFlashMS) {
        std::string msg = "light countdown stoped too long, light_id=" +
                          std::to_string(unsigned(kv.first)) + ", stoped ms=" +
                          std::to_string(unsigned(time_ms_delta));
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "" << msg;
      }

      // 倒计时异常（非法跳变）
      if ((one_old_state.countdown == one_new_state.countdown) ||
          ((one_new_state.countdown + 1) == one_old_state.countdown)) {
        // 倒计时无变化（正常）
        // 当前帧倒计时与上一帧倒计时相差1秒（正常）
      } else {
        if (one_new_state.color != one_old_state.color) {  // 灯色改变
          if ((one_new_state.color != 0) && (!cur_curr_period.empty())) {
            uint8_t tmp_color     = (one_new_state.color > 10)
                                        ? (one_new_state.color - 10)
                                        : one_new_state.color;
            uint16_t tmp_duration = 0;
            std::string tmp_steps_str;
            for (auto &one_step : cur_curr_period[kv.first]) {
              tmp_steps_str +=
                  "(" + std::to_string(unsigned(one_step.color)) + "," +
                  std::to_string(unsigned(one_step.duration)) + ") ";
              uint8_t tmp_step_color = (one_step.color > 10)
                                           ? (one_step.color - 10)
                                           : one_step.color;
              if (tmp_step_color == tmp_color) {
                tmp_duration += one_step.duration;
              }
            }
            if (tmp_duration !=
                one_new_state
                    .countdown) {  // 倒计时未从周期起始时间开始（异常）
              std::string msg =
                  "light countdown start invalid, light_id=" +
                  std::to_string(unsigned(kv.first)) + ", prev(countdown=" +
                  std::to_string(unsigned(one_old_state.countdown)) +
                  ",color=" + std::to_string(unsigned(one_old_state.color)) +
                  "), now(countdown=" +
                  std::to_string(unsigned(one_new_state.countdown)) +
                  ",color=" + std::to_string(unsigned(one_new_state.color)) +
                  "), steps=(" + tmp_steps_str + ")";
              TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "" << msg;
            }
          }
        } else {  // 灯色未改变
          // 灯色倒计时跳变（异常）
          std::string msg =
              "light countdown change invalid, light_id=" +
              std::to_string(unsigned(kv.first)) +
              ", prev=" + std::to_string(unsigned(one_old_state.countdown)) +
              ", now=" + std::to_string(unsigned(one_new_state.countdown));
          TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "" << msg;
        }
      }
    }
  }
}

void GatMonitor::CheckCurrPlanStep(
    const std::map<uint8_t, std::vector<GatLightStep>> &curr_plan_step) {
  // 检查各个周期时长是否一致
  uint16_t total_duration = 0;
  for (auto &one_light_kv : curr_plan_step) {
    uint16_t tmp_total_duration = 0;
    for (auto &one_step : one_light_kv.second) {
      tmp_total_duration += one_step.duration;
    }
    if (total_duration == 0) {
      total_duration = tmp_total_duration;
    } else {
      if (total_duration != tmp_total_duration) {
        std::string msg =
            "light curr plan step total duration not same, id_1 duration=" +
            std::to_string(unsigned(total_duration)) + ", id_" +
            std::to_string(unsigned(one_light_kv.first)) +
            " duration=" + std::to_string(unsigned(tmp_total_duration));
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "" << msg;
      }
    }
  }
}

void GatMonitor::UpdateTimestampGatPacket() {
  timestamp_gat_packet_ = GatUtil::GetCurTimestampMsec();
}

void GatMonitor::UpdateTimestampColorState() {
  timestamp_color_state_ = GatUtil::GetCurTimestampMsec();
}

void GatMonitor::UpdateTimestampCurrPlanStep() {
  timestamp_curr_plan_step_ = GatUtil::GetCurTimestampMsec();
}

bool GatMonitor::InitTimerColorState() {
  struct sigevent evp;
  evp.sigev_notify            = SIGEV_THREAD;
  evp.sigev_notify_function   = GatMonitor::TimeoutColorState;
  evp.sigev_value.sival_ptr   = this;
  evp.sigev_notify_attributes = NULL;

  if (0 != timer_create(CLOCK_REALTIME, &evp, &timer_fd_color_state_)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer create failed.";
    return false;
  }

  struct itimerspec ts;
  ts.it_interval.tv_sec  = kColorStateSamplingSec;
  ts.it_interval.tv_nsec = 0;
  ts.it_value.tv_sec     = kColorStateSamplingSec;
  ts.it_value.tv_nsec    = 0;

  if (0 != timer_settime(timer_fd_color_state_, 0, &ts, NULL)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer set failed.";
    return false;
  }

  return true;
}

bool GatMonitor::InitTimerCurrPlanStep() {
  struct sigevent evp;
  evp.sigev_notify            = SIGEV_THREAD;
  evp.sigev_notify_function   = GatMonitor::TimeoutCurrPlanStep;
  evp.sigev_value.sival_ptr   = this;
  evp.sigev_notify_attributes = NULL;

  if (0 != timer_create(CLOCK_REALTIME, &evp, &timer_fd_curr_plan_step_)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer create failed.";
    return false;
  }

  struct itimerspec ts;
  ts.it_interval.tv_sec  = kCurrPlanStepSamplingSec;
  ts.it_interval.tv_nsec = 0;
  ts.it_value.tv_sec     = kCurrPlanStepSamplingSec;
  ts.it_value.tv_nsec    = 0;

  if (0 != timer_settime(timer_fd_curr_plan_step_, 0, &ts, NULL)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer set failed.";
    return false;
  }

  return true;
}

}  // namespace device
}  // namespace v2x
}  // namespace os
