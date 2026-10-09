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

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "base/device_connect/proto/traffic_light_data.pb.h"
#include "base/device_connect/traffic_light/traffic_flow_data.h"

#include "gat_base_type.h"
#include "gat_monitor.h"
#include <iostream>
#include <iomanip>
#include <arpa/inet.h>
namespace os {
namespace v2x {
namespace device {

struct outAddress
{
	int peerport;
	std::string peerip;
};

/**
 * @brief 协议解析器类
 * @note 解析和构造GAT1743协议报文
 */
class GatParser {
 public:
  GatParser();
  ~GatParser();

  bool Init(std::shared_ptr<GatMonitor> &monitor, std::vector<outAddress> list);

  bool GetTrafficLightData(os::v2x::device::TrafficLightBaseData &lamp_info);

  void ProcessFrames(uint8_t *frame_buf, const int frame_len);

  size_t MakePacketQueryColorState(uint8_t *packet_buf, int packet_len);
  size_t MakePacketQueryCurrPlanStep(uint8_t *packet_buf, int packet_len);
  size_t MakePacketPulse(uint8_t *packet_buf, int buf_len,
                         const std::vector<std::pair<uint8_t, uint8_t>> &lanes);
  size_t MakePacketTrafficFlow(uint8_t *packet_buf, int buf_len,
                               const std::vector<CoilFlowData> &coils);

  static void TimeoutSendSourcedate(__sigval_t arg);
 private:
  uint16_t BytesToU16(uint8_t *data);
  uint32_t BytesToU32(uint8_t *data);
  uint16_t GatCrc16(uint8_t *data, uint32_t data_len);
  int RemoveEscapeChar(uint8_t *packet_buf, int packet_len);

  size_t MakeQueryPacket(uint8_t *packet_addr, uint16_t obj_flag);

  bool ParseColorState(uint8_t *packet_addr, int packet_len);
  bool ParseCurrPlanStep(uint8_t *packet_addr, int packet_len);
  bool ParseRunState(uint8_t *packet_addr, int packet_len);
  bool ParseControlMode(uint8_t *packet_addr, int packet_len);
  void ParseOnePacket(uint8_t *packet_addr, int packet_len);

  void ResetAllColorState(std::map<uint8_t, GatLightState> &new_color_state);
  void ResetAllCurrPlanStep(
      std::map<uint8_t, std::vector<GatLightStep>> &new_curr_plan_step);

  uint8_t CovertGatLightColor(uint8_t gat_light_color);

  void JudgeProcessSeconds();

  void GetAllColorState(std::map<uint8_t, GatLightState> &color_state);
  void SetAllColorState(std::map<uint8_t, GatLightState> &color_state);
  size_t GetAllColorStateSize();
  void GetAllCurrPlanStep(
      std::map<uint8_t, std::vector<GatLightStep>> &curr_plan_step);
  void SetAllCurrPlanStep(
      std::map<uint8_t, std::vector<GatLightStep>> &curr_plan_step);
  size_t GetAllCurrPlanStepSize();

  bool IsDataReady();

  LightState PbCovertLightColor(uint8_t light_color);
  LightType PbCovertLightType(uint8_t light_type);
private:

  void sendSourceDataToCloud(uint8_t *package_buf, const int package_len);
  const uint32_t kAdministrativeCode =
      110113;  // 行政区代码 北京顺义区(GB/T2260 - 2007)

  std::map<uint8_t, GatLightState> all_color_state_;  // 全灯组灯色状态
  std::mutex all_color_state_mutex_;
  std::map<uint8_t, std::vector<GatLightStep>>
      all_curr_plan_step_;  //  全灯组当前方案色步信息
  std::mutex all_curr_plan_step_mutex_;

  std::queue<std::string> gat_packet_;  // GAT1743原始通信报文帧
  std::queue<std::pair<int, std::string>> gat_packet_src_;  // GAT1743原始通信报文帧
  std::mutex gat_packet_mutex_;

  uint16_t total_period_ = 0;      // 总周期（秒数）
  uint16_t progress_seconds_ = 0;  // 周期内已经进行秒数（从1开始）
  uint8_t control_mode_  = 0;      // 控制模式(GAT-1743)
  uint8_t running_state_ = 0;      // 运行状态(GAT-1743)

  uint8_t *recv_cache_          = nullptr;
  int cache_size_               = 0;
  const int kRecvCacheCapacity  = 2048;
  uint8_t *packet_buff_         = nullptr;
  const int kPacketBuffCapacity = 1024;

  uint32_t sequence_num_ = 0;  // 消息序列码

  std::shared_ptr<GatMonitor> monitor_ = nullptr;
  int socket_fd_              = -1;
  std::vector<outAddress> outlist;

  timer_t timer_send_source_data_    = 0;
  uint64_t source_data_update_time_ = 0;
  std::string last_source_date_ = "";
};

}  // namespace device
}  // namespace v2x
}  // namespace os
