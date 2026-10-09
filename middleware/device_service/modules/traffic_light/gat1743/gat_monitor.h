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

#include <signal.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include <map>
#include <mutex>
#include <vector>

#include "gat_base_type.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @brief 信号机监控类
 * @note 监控和信号机的通信是否正常
 */
class GatMonitor {
 public:
  GatMonitor();
  ~GatMonitor();

  bool Init();

  void IncRecvFrameCounter();

  void IncGatPacketCounter();
  void IncGatBadPacketCounter();

  float GetColorStatePacketFreq();
  void ZeroColorStatePacketCounter();

  float GetCurrPlanStepPacketFreq();
  void ZeroCurrPlanStepPacketCounter();

  static void TimeoutColorState(__sigval_t arg);
  static void TimeoutCurrPlanStep(__sigval_t arg);

  bool IsDataExpired();
  bool IsRemoteAlive();

  void FlushColorState(std::map<uint8_t, GatLightState> &color_state);
  void FlushCurrPlanStep(
      std::map<uint8_t, std::vector<GatLightStep>> &curr_plan_step);

 private:
  void CheckColorState(const std::map<uint8_t, GatLightState> &color_state);
  void CheckCurrPlanStep(
      const std::map<uint8_t, std::vector<GatLightStep>> &curr_plan_step);

  void IncColorStatePacketCounter();
  void IncCurrPlanStepPacketCounter();

  void UpdateTimestampGatPacket();
  void UpdateTimestampColorState();
  void UpdateTimestampCurrPlanStep();

  bool InitTimerColorState();
  bool InitTimerCurrPlanStep();

  uint64_t recv_frame_counter_     = 0;      // 接收数据帧计数器
  uint64_t gat_packet_counter_     = 0;      // GAT1743报文包计数器
  uint64_t gat_bad_packet_counter_ = 0;      // GAT1743非法报文包计数器
  uint64_t color_state_packet_counter_ = 0;  // 灯色状态报文包接收计数器
  uint64_t curr_plan_step_packet_counter_ =
      0;  // 当前方案色步信息报文包接收计数器

  const int kColorStateSamplingSec = 1;  // 灯色状态报文包采样秒数
  const int kCurrPlanStepSamplingSec = 5;  // 当前方案色步信息报文包接收采样秒数

  timer_t timer_fd_color_state_    = nullptr;
  timer_t timer_fd_curr_plan_step_ = nullptr;

  uint64_t timestamp_gat_packet_ = 0;  // GAT1743报文包接收刷新时间
  uint64_t timestamp_color_state_ = 0;  // 灯色状态报文包接收刷新时间
  uint64_t timestamp_curr_plan_step_ = 0;  // 当前方案色步信息报文包接收刷新时间

  const uint64_t kColorStateExpireMS = 2000;  // 灯色状态失效时间
  const uint64_t kGatPacketExpireMS  = 3000;  // GAT1743报文包失效时间
  const uint64_t kColorStateFlashMS  = 1300;  // 灯色状态刷新超时时间

  std::map<uint8_t, GatLightState> color_state_;  // 全灯组灯色状态
  std::mutex color_state_mutex_;
  std::map<uint8_t, std::vector<GatLightStep>>
      curr_plan_step_;  // 全灯组当前方案色步信息
  std::mutex curr_plan_step_mutex_;
};

}  // namespace device
}  // namespace v2x
}  // namespace os
