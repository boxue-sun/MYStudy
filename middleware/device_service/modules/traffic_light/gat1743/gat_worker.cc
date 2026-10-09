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

#include "gat_worker.h"

#include <sys/time.h>

#include <functional>
#include <iostream>
#include <sstream>

#include "base/common/log.h"

namespace os {
namespace v2x {
namespace device {

GatWorker::~GatWorker() {
  this->Stop();
}

bool GatWorker::Init(
    const std::string &remote_ip, const uint16_t remote_port,
    const std::string &host_ip, const uint16_t host_port,
    const std::string &protocol,
	  std::vector<outAddress> list,
    double color_state_query_cycle) {
  GatCommunication::ProtocolType protocol_type;
  if (protocol == "tcp") {
    protocol_type = GatCommunication::ProtocolType::TCP;
  } else if (protocol == "udp") {
    protocol_type = GatCommunication::ProtocolType::UDP;
  } else {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "not support this protocol: " << protocol;
    return false;
  }
  color_state_query_sec = (int)color_state_query_cycle;
  color_state_query_ms = (int)((color_state_query_cycle - color_state_query_sec)*1000);
  communication_.reset(new GatCommunication());
  if (!communication_->Init(
          remote_ip, remote_port, host_ip, host_port, protocol_type)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "GatCommunication init error";
    return false;
  }
  monitor_.reset(new GatMonitor());
  if (!monitor_->Init()) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "GatMonitor init error";
    return false;
  }

  parser_.reset(new GatParser());
  if (!parser_->Init(monitor_, list)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "GatParser init error";
    return false;
  }

  thread_process_recv_.reset(
      new std::thread(std::bind(&GatWorker::TaskProcessRecvFrame, this)));

  if (!this->InitTimerQueryColorState()) {
    return false;
  }

  // if (!this->InitTimerQueryCurrPlanStep()) {
  //   return false;
  // }

  if (!this->InitTimerKeepConnection()) {
    return false;
  }

  return true;
}

void GatWorker::Stop() {
  stop_ = true;

  if (timer_query_color_state_ != nullptr) {
    timer_delete(timer_query_color_state_);
  }

  if (timer_query_curr_plan_step_ != nullptr) {
    timer_delete(timer_query_curr_plan_step_);
  }

  if (timer_keep_connection_ != nullptr) {
    timer_delete(timer_keep_connection_);
  }

  if (thread_process_recv_ != nullptr && thread_process_recv_->joinable()) {
    thread_process_recv_->join();
  }
}

bool GatWorker::GetTrafficLightData(
    os::v2x::device::TrafficLightBaseData &lamp_info) {
  return parser_->GetTrafficLightData(lamp_info);
}

void GatWorker::TimeoutQueryColorState(__sigval_t arg) {
  GatWorker *worker = reinterpret_cast<GatWorker *>(arg.sival_ptr);

  uint8_t send_buff[80] = {0};

  size_t send_len =
      worker->parser_->MakePacketQueryColorState(send_buff, sizeof(send_buff));

  ssize_t ret_val = worker->SendFrame(send_buff, send_len);
  if (ret_val < 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "send query light state error.";
  }
}

void GatWorker::TimeoutQueryCurrPlanStep(__sigval_t arg) {
  GatWorker *worker = reinterpret_cast<GatWorker *>(arg.sival_ptr);

  uint8_t send_buff[80] = {0};

  size_t send_len = worker->parser_->MakePacketQueryCurrPlanStep(
      send_buff, sizeof(send_buff));

  ssize_t ret_val = worker->SendFrame(send_buff, send_len);
  if (ret_val < 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "send query curr period error.";
  }
}

void GatWorker::TimeoutKeepConnection(__sigval_t arg) {
  GatWorker *worker = reinterpret_cast<GatWorker *>(arg.sival_ptr);

  if (!worker->monitor_->IsRemoteAlive()) {
    if (!worker->communication_->Connect()) {
      TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "connect error.";
    }
  }
}

ssize_t GatWorker::SendFrame(uint8_t *packet_addr, size_t packet_len) {
  return communication_->SendData(packet_addr, packet_len);
}

void GatWorker::SendPulseData(
    const std::vector<std::pair<uint8_t, uint8_t>> &lanes) {
  // 最大 256 通道，每通道 2B，加帧头开销预留 1024B
  uint8_t send_buff[1024] = {0};
  size_t send_len =
      parser_->MakePacketPulse(send_buff, sizeof(send_buff), lanes);
  if (send_len == 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "MakePacketPulse failed, lanes=" << lanes.size();
    return;
  }
  ssize_t ret = SendFrame(send_buff, send_len);
  if (ret < 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "send pulse data error.";
  }
}

void GatWorker::SendTrafficFlowData(
    const std::vector<os::v2x::device::CoilFlowData> &coils) {
  // 帧头开销36B + 通道数1B + 每通道10B，预留1024B
  uint8_t send_buff[1024] = {0};
  size_t send_len =
      parser_->MakePacketTrafficFlow(send_buff, sizeof(send_buff), coils);
  if (send_len == 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "MakePacketTrafficFlow failed, coils=" << coils.size();
    return;
  }
  ssize_t ret = SendFrame(send_buff, send_len);
  if (ret < 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "send trafficflow data error.";
  }
}

void GatWorker::TaskProcessRecvFrame() {
  TRAFFIC_LIFGT_SERVICE_LOG_WARN << "start task : process response.";
  uint8_t recv_buff[2048] = {0};
  ssize_t recv_len        = -1;
  while (!stop_) {
    recv_len = communication_->RecvDataWait(recv_buff, sizeof(recv_buff));
    if (recv_len > 0) {
      parser_->ProcessFrames(recv_buff, recv_len);
    }
  }

  TRAFFIC_LIFGT_SERVICE_LOG_WARN << "task over : process response.";
}

bool GatWorker::InitTimerQueryColorState() {
  struct sigevent evp;
  evp.sigev_notify            = SIGEV_THREAD;
  evp.sigev_notify_function   = GatWorker::TimeoutQueryColorState;
  evp.sigev_value.sival_ptr   = this;
  evp.sigev_notify_attributes = NULL;

  if (0 != timer_create(CLOCK_REALTIME, &evp, &timer_query_color_state_)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer create failed.";
    return false;
  }

  struct itimerspec ts;
  ts.it_interval.tv_sec  = color_state_query_sec;
  ts.it_interval.tv_nsec = color_state_query_ms * 1000000L;
  ts.it_value.tv_sec     = color_state_query_sec;
  ts.it_value.tv_nsec    = color_state_query_ms * 1000000L;

  if (0 != timer_settime(timer_query_color_state_, 0, &ts, NULL)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer set failed.";
    return false;
  }

  return true;
}

bool GatWorker::InitTimerQueryCurrPlanStep() {
  struct sigevent evp;
  evp.sigev_notify            = SIGEV_THREAD;
  evp.sigev_notify_function   = GatWorker::TimeoutQueryCurrPlanStep;
  evp.sigev_value.sival_ptr   = this;
  evp.sigev_notify_attributes = NULL;

  if (0 != timer_create(CLOCK_REALTIME, &evp, &timer_query_curr_plan_step_)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer create failed.";
    return false;
  }

  struct itimerspec ts;
  ts.it_interval.tv_sec  = 1;
  ts.it_interval.tv_nsec = 0;
  ts.it_value.tv_sec     = 1;
  ts.it_value.tv_nsec    = 0;

  if (0 != timer_settime(timer_query_curr_plan_step_, 0, &ts, NULL)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer set failed.";
    return false;
  }

  return true;
}

bool GatWorker::InitTimerKeepConnection() {
  struct sigevent evp;
  evp.sigev_notify            = SIGEV_THREAD;
  evp.sigev_notify_function   = GatWorker::TimeoutKeepConnection;
  evp.sigev_value.sival_ptr   = this;
  evp.sigev_notify_attributes = NULL;

  if (0 != timer_create(CLOCK_REALTIME, &evp, &timer_keep_connection_)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer create failed.";
    return false;
  }

  struct itimerspec ts;
  ts.it_interval.tv_sec  = 3;
  ts.it_interval.tv_nsec = 0;
  ts.it_value.tv_sec     = 3;
  ts.it_value.tv_nsec    = 0;

  if (0 != timer_settime(timer_keep_connection_, 0, &ts, NULL)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer set failed.";
    return false;
  }

  return true;
}

}  // namespace device
}  // namespace v2x
}  // namespace os
