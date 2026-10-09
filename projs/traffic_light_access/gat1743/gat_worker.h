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

#include <memory>
#include <string>
#include <thread>

#include "gat_communication.h"
#include "gat_monitor.h"
#include "gat_parser.h"

namespace os {
namespace v2x {
namespace device {

class GatWorker {
 public:
  GatWorker() = default;
  ~GatWorker();

  bool Init(
      const std::string &remote_ip, const uint16_t remote_port,
      const std::string &host_ip, const uint16_t host_port,
      const std::string &protocol,
	  std::vector<outAddress> list);
  void Stop();
  bool GetTrafficLightData(os::v2x::device::TrafficLightBaseData &lamp_info);

  static void TimeoutQueryColorState(__sigval_t arg);
  static void TimeoutQueryCurrPlanStep(__sigval_t arg);
  static void TimeoutKeepConnection(__sigval_t arg);

  ssize_t SendFrame(uint8_t *packet_addr, size_t packet_len);

  void TaskProcessRecvFrame();

  std::shared_ptr<GatCommunication> communication_ = nullptr;
  std::shared_ptr<GatMonitor> monitor_             = nullptr;
  std::shared_ptr<GatParser> parser_               = nullptr;

 private:
  bool InitTimerQueryColorState();
  bool InitTimerQueryCurrPlanStep();
  bool InitTimerKeepConnection();

  std::unique_ptr<std::thread> thread_process_recv_ = nullptr;

  timer_t timer_query_color_state_    = nullptr;
  timer_t timer_query_curr_plan_step_ = nullptr;
  timer_t timer_keep_connection_      = nullptr;
  volatile bool stop_                 = false;

  const int kQueryColorStateInterval =
      120;  // 主动发送查询灯色状态时间间隔,单位:毫秒,期望频率8~10HZ(100ms~125ms);
};

}  // namespace device
}  // namespace v2x
}  // namespace os
