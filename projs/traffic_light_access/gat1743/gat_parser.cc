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

#include "gat_parser.h"

#include <cmath>
#include <functional>

#include "base/common/log.h"
#include "gat_util.h"

namespace os {
namespace v2x {
namespace device {

GatParser::GatParser() {
  recv_cache_ = new uint8_t[kRecvCacheCapacity];
  memset(recv_cache_, 0, kRecvCacheCapacity);
  cache_size_ = 0;

  packet_buff_ = new uint8_t[kPacketBuffCapacity];
  memset(packet_buff_, 0, kPacketBuffCapacity);
}
GatParser::~GatParser() {
  if (recv_cache_ != nullptr) {
    delete[] recv_cache_;
  }

  if (packet_buff_ != nullptr) {
    delete[] packet_buff_;
  }
}

bool GatParser::Init(std::shared_ptr<GatMonitor> &monitor, std::vector<outAddress> list) {

  for(int i = 0; i < list.size(); i++)
  {
	  outAddress oa;
	  oa.peerip = list[i].peerip;
	  oa.peerport = list[i].peerport;

	  outlist.push_back(oa);
  }

  socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd_ < 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "cloud udp socket create error, errno=" << strerror(errno);
  }
  else
  {
	TRAFFIC_LIFGT_SERVICE_LOG_INFO << "cloud udp socket creat success";
  }

  monitor_ = monitor;
  return true;
}

// 获取信号机的实时状态信息
bool GatParser::GetTrafficLightData(
    os::v2x::device::TrafficLightBaseData &lamp_info) {
  lamp_info.Clear();

  {  // 装填信号机通信的原始报文帧
    std::unique_lock<std::mutex> guard(gat_packet_mutex_);
    while (!gat_packet_.empty()) {
      lamp_info.add_original_data_list(gat_packet_.front());
            std::pair<int, std::string> src_data_pair = gat_packet_src_.front();
            lamp_info.set_src_data(src_data_pair.second);
            lamp_info.set_mess_type(src_data_pair.first);
            TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "[mess_type]" << src_data_pair.first;
            TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "[src_data]" << src_data_pair.second;
            gat_packet_src_.pop();
      gat_packet_.pop();
    }
  }

  if (!this->IsDataReady()) {
    return false;
  }

  lamp_info.set_time_stamp(GatUtil::GetCurTimestampMsec());  // 当前时间戳
  lamp_info.set_sequence_num(sequence_num_++);               // 消息序列号
  lamp_info.set_data_source(os::v2x::device::SIGNAL);        // 信号机直连
  lamp_info.set_work_status(os::v2x::device::DEV_NORMAL);    // 工作状态
  lamp_info.set_period(total_period_);  // 周期的总倒计时时长
  lamp_info.set_period_count_down(
      total_period_ - progress_seconds_ + 1);  // 周期内倒计时

  std::map<uint8_t, GatLightState> tmp_all_color_state;
  this->GetAllColorState(tmp_all_color_state);
  std::map<uint8_t, std::vector<GatLightStep>> tmp_all_light_period;
  this->GetAllCurrPlanStep(tmp_all_light_period);

  lamp_info.set_control_mode(os::v2x::device::LOCAL_MANUAL);  // 手控模式
  lamp_info.set_confidence(0.5);  // 手控模式下置信度降低
  for (auto &kv : tmp_all_color_state) {
    if (kv.second.countdown != 0) {
      lamp_info.set_control_mode(
          os::v2x::device::LOCAL_FIX_CYCLE);  // 判定为定周期控制模式
      lamp_info.set_confidence(1);  // 固定周期模式下置信度最高
    }
  }

  // 特殊情形：只有灯态没有周期的情况下，设置其它字段缺省值
  if (!tmp_all_color_state.empty() && tmp_all_light_period.empty()) {
    lamp_info.set_control_mode(
        os::v2x::device::LOCAL_MANUAL);  // 这种情形下，暂定视为手控模式
    lamp_info.set_confidence(0.5);  // 特殊情形下置信度降低
  }

  uint8_t tmp_light_id   = 0;
  uint8_t tmp_count_down = 0;
  uint8_t tmp_color      = 0;
  uint32_t tmp_entry_direction = 0;

  // 融合灯态倒计时和周期，计算当前色步，下个色步，下下个色步
  for (auto &one_light_state_kv : tmp_all_color_state) {
    GatLightState &one_light_state = one_light_state_kv.second;
    tmp_count_down                 = one_light_state.countdown;
    tmp_color                      = one_light_state.color;
    if (!((tmp_color == 1) || (tmp_color == 11) || (tmp_color == 2) ||
          (tmp_color == 12) || (tmp_color == 3) ||
          (tmp_color == 13))) {  // 非正常灯色忽略之
      continue;
    }
    tmp_entry_direction            = one_light_state.entry_direction;

    OneLightInfo *one_light_info_ptr = lamp_info.add_light_info_list();
    // 灯组号
    tmp_light_id = one_light_state_kv.first;  
    one_light_info_ptr->set_light_id(::google::protobuf::int32(tmp_light_id));
    // 灯组类型
    one_light_info_ptr->set_light_type(PbCovertLightType(one_light_state.type));
    // 当前灯态
    LightState tmp_cur_light_color = PbCovertLightColor(tmp_color);
    one_light_info_ptr->set_light_status(tmp_cur_light_color);

    // 进口道方向 暂时注释掉
    one_light_info_ptr->set_entry_direction(tmp_entry_direction);

    // 当前灯态结束倒计时
    if (0 != tmp_count_down) {
      // GAT1743中规定倒计时不能为0，因此，当信号机发来的倒计时为0时则认为倒计时无效，此时不装填倒计时参数。
      one_light_info_ptr->set_count_down(
          ::google::protobuf::int32(tmp_count_down));
    }

    if (tmp_all_light_period.count(tmp_light_id) ==
        0) {  // 无匹配的周期，则设置缺省路权时间和步色信息
      TRAFFIC_LIFGT_SERVICE_LOG_WARN << "light period not exist, id = " << unsigned(tmp_light_id);
      // 路权时间
      one_light_info_ptr->set_right_way_time(0);
      // 当前、下个以及下下个步色和周期
      LightState tmp_light_color[3] = {os::v2x::device::DARK};
      if ((tmp_cur_light_color == os::v2x::device::RED) ||
          (tmp_cur_light_color == os::v2x::device::FLASHING_RED)) {
        tmp_light_color[0] = os::v2x::device::RED;
        tmp_light_color[1] = os::v2x::device::GREEN;
        tmp_light_color[2] = os::v2x::device::YELLOW;
      } else if (
          (tmp_cur_light_color == os::v2x::device::GREEN) ||
          (tmp_cur_light_color == os::v2x::device::FLASHING_GREEN)) {
        tmp_light_color[0] = os::v2x::device::GREEN;
        tmp_light_color[1] = os::v2x::device::YELLOW;
        tmp_light_color[2] = os::v2x::device::RED;
      } else if (
          (tmp_cur_light_color == os::v2x::device::YELLOW) ||
          (tmp_cur_light_color == os::v2x::device::FLASHING_YELLOW)) {
        tmp_light_color[0] = os::v2x::device::YELLOW;
        tmp_light_color[1] = os::v2x::device::RED;
        tmp_light_color[2] = os::v2x::device::GREEN;
      } else if (tmp_cur_light_color == os::v2x::device::DARK) {
        tmp_light_color[0] = os::v2x::device::DARK;
        tmp_light_color[1] = os::v2x::device::DARK;
        tmp_light_color[2] = os::v2x::device::DARK;
      } else {
        tmp_light_color[0] = os::v2x::device::UNAVAILABLE;
        tmp_light_color[1] = os::v2x::device::UNAVAILABLE;
        tmp_light_color[2] = os::v2x::device::UNAVAILABLE;
      }
      for (int i = 0; i < 3; i++) {
        ::os::v2x::device::OneStepInfo *one_step_info_ptr =
            one_light_info_ptr->add_step_info_list();
        one_step_info_ptr->set_light_status(tmp_light_color[i]);
        one_step_info_ptr->set_duration(0);
      }

      continue;
    }

    std::vector<GatLightStep> &tmp_one_light_period =
        tmp_all_light_period[tmp_light_id];
    // 路权时间
    int tmp_way_right_time = 0;
    for (auto &one_step : tmp_one_light_period) {
      if ((one_step.color != 1) && (one_step.color != 11)) {
        tmp_way_right_time += one_step.duration;
      }
    }
    one_light_info_ptr->set_right_way_time(tmp_way_right_time);

    // 定位当前步色周期，确定下个以及下下个步色和周期
    if ((progress_seconds_ != 0) &&
        (tmp_count_down != 0)) {  // 根据行进秒数确定下个以及下下个态
      uint16_t tmp_total_time = 0;
      const int max_i(tmp_one_light_period.size());
      for (int i = 0; i < max_i; i++) {
        tmp_total_time += tmp_one_light_period[i].duration;
        if (tmp_total_time >= progress_seconds_) {  // 定位到当前色步
          // 当前步色信息
          ::os::v2x::device::OneStepInfo *one_step_info_ptr =
              one_light_info_ptr->add_step_info_list();  // 当前步色信息
          one_step_info_ptr->set_light_status(PbCovertLightColor(tmp_color));
          uint16_t tmp_duration = tmp_one_light_period[i].duration;
          int next_i            = (i + 1) % max_i;
          if (tmp_one_light_period[i].color ==
              tmp_one_light_period[next_i].color) {
            // 当前步色和下一个步色颜色一致时，周期累加合并
            tmp_duration += tmp_one_light_period[next_i].duration;
            next_i = (next_i + 1) % max_i;
          }
          one_step_info_ptr->set_duration(
              ::google::protobuf::int32(tmp_duration));

          // 下一个步色信息
          ::os::v2x::device::OneStepInfo *two_step_info_ptr =
              one_light_info_ptr->add_step_info_list();  // 下一步色信息
          two_step_info_ptr->set_light_status(
              PbCovertLightColor(tmp_one_light_period[next_i].color));
          tmp_duration    = tmp_one_light_period[next_i].duration;
          int next_next_i = (next_i + 1) % max_i;
          if (tmp_one_light_period[next_i].color ==
              tmp_one_light_period[next_next_i].color) {
            // 下一个步色和下下个步色一致时，周期累加合并
            tmp_duration += tmp_one_light_period[next_next_i].duration;
            next_next_i = (next_next_i + 1) % max_i;
          }
          two_step_info_ptr->set_duration(
              ::google::protobuf::int32(tmp_duration));
          // 下下一步色信息
          ::os::v2x::device::OneStepInfo *three_step_info_ptr =
              one_light_info_ptr->add_step_info_list();  // 下下一步色信息
          three_step_info_ptr->set_light_status(
              PbCovertLightColor(tmp_one_light_period[next_next_i].color));

          tmp_duration         = tmp_one_light_period[next_next_i].duration;
          int next_next_next_i = (next_next_i + 1) % max_i;
          if (tmp_one_light_period[next_next_i].color ==
              tmp_one_light_period[next_next_next_i].color) {
            // 下下一个步色和下下下个步色一致时，周期累加合并
            tmp_duration += tmp_one_light_period[next_next_next_i].duration;
          }
          three_step_info_ptr->set_duration(
              ::google::protobuf::int32(tmp_duration));
          break;  // 结束，去处理下一个灯
        }
      }
    } else {  // 周期游标刻度无效时（行进秒数为0或者倒计时为0），根据灯色模糊确定下个以及下下个态(如进入手控模式后)
      uint16_t tmp_total_time = 0;
      const int max_i(tmp_one_light_period.size());
      for (int i = 0; i < max_i; i++) {
        tmp_total_time += tmp_one_light_period[i].duration;
        if (tmp_one_light_period[i].color == tmp_color) {  // 定位到当前色步
          // 当前步色信息
          ::os::v2x::device::OneStepInfo *one_step_info_ptr =
              one_light_info_ptr->add_step_info_list();  // 当前步色信息
          one_step_info_ptr->set_light_status(PbCovertLightColor(tmp_color));
          uint16_t tmp_duration = tmp_one_light_period[i].duration;
          int next_i            = (i + 1) % max_i;
          if (tmp_one_light_period[i].color ==
              tmp_one_light_period[next_i].color) {
            // 当前步色和下一个步色颜色一致时，周期累加合并
            tmp_duration += tmp_one_light_period[next_i].duration;
            next_i = (next_i + 1) % max_i;
          }
          one_step_info_ptr->set_duration(
              ::google::protobuf::int32(tmp_duration));

          // 下一步色信息
          ::os::v2x::device::OneStepInfo *two_step_info_ptr =
              one_light_info_ptr->add_step_info_list();  // 下一步色信息
          two_step_info_ptr->set_light_status(
              PbCovertLightColor(tmp_one_light_period[next_i].color));
          tmp_duration    = tmp_one_light_period[next_i].duration;
          int next_next_i = (next_i + 1) % max_i;
          if (tmp_one_light_period[next_i].color ==
              tmp_one_light_period[next_next_i].color) {
            // 下一个步色和下下个步色一致时，周期累加合并
            tmp_duration += tmp_one_light_period[next_next_i].duration;
            next_next_i = (next_next_i + 1) % max_i;
          }
          two_step_info_ptr->set_duration(
              ::google::protobuf::int32(tmp_duration));

          // 下下一步色信息
          ::os::v2x::device::OneStepInfo *three_step_info_ptr =
              one_light_info_ptr->add_step_info_list();  // 下下一步色信息
          three_step_info_ptr->set_light_status(
              PbCovertLightColor(tmp_one_light_period[next_next_i].color));
          tmp_duration         = tmp_one_light_period[next_next_i].duration;
          int next_next_next_i = (next_next_i + 1) % max_i;
          if (tmp_one_light_period[next_next_i].color ==
              tmp_one_light_period[next_next_next_i].color) {
            // 下下一个步色和下下下个步色一致时，周期累加合并
            tmp_duration += tmp_one_light_period[next_next_next_i].duration;
          }
          three_step_info_ptr->set_duration(
              ::google::protobuf::int32(tmp_duration));
          break;  // 结束，去处理下一个灯
        }
      }
    }
  }

  return true;
}

// @brief 拆解叠加包：一个包中存在粘连在一起的多个完整的数据帧
//      case1: GAT + GAT
//      case2: GAT + Hisense
//      case3: Hisense + GAT
// @param frame_buf 叠包首地址
// @param frame_len 叠包数据总长度
void GatParser::ProcessFrames(uint8_t *package_buf, const int package_len) {
  monitor_->IncRecvFrameCounter();

  if (cache_size_ + package_len > kRecvCacheCapacity) {
    cache_size_ = 0;  // clean cache
    GatUtil::DbgPrintBinary(recv_cache_, cache_size_, "clean cache");
    for (int i = 0; i < package_len; i++) {
      if ((package_buf[i] == 0xC0) && (package_buf[i + 1] != 0xC0)) {
        // find the FrameHead
        if ((package_len - i) <= kRecvCacheCapacity) {
          memcpy(recv_cache_, &package_buf[i], package_len - i);
          cache_size_ = package_len - i;
        } else {
          GatUtil::DbgPrintBinary(
              package_buf, package_len, "drop the headless frame: ");
        }
      }
    }
  } else {
    memcpy(recv_cache_ + cache_size_, package_buf, package_len);
    cache_size_ += package_len;
  }

  if (cache_size_ == 0) {
    return;
  }

  // pick up all gat frames in the chche
  std::map<uint8_t *, int> gat_packet_map;  // frame addr : frame len
    std::map < uint8_t * , int > gat_packet_src_map;  // frame addr : frame len
  int shear_len       = 0;
  const int max_end   = cache_size_;
  uint8_t *frame_head = nullptr;

  for (int offset = 0; offset < max_end; offset++) {
    if (recv_cache_[offset] == 0xC0) {
      if (offset + 1 == max_end) {
        if (frame_head != nullptr) {  // pick the last gat frame
          gat_packet_map[frame_head] =
              int(&recv_cache_[offset] - frame_head + 1);
          frame_head = nullptr;
          shear_len  = offset + 1;
        }
      } else {
        if (recv_cache_[offset + 1] != 0xC0) {  // FrameHead
          if (frame_head != nullptr) {
            shear_len = offset;
            GatUtil::DbgPrintBinary(
                recv_cache_, cache_size_, "drop the tailless frame: ");
          }
          frame_head = &recv_cache_[offset];
        } else {  // FrameTail
          shear_len = offset + 1;
          if (frame_head != nullptr) {  // pick the last gat frame
            gat_packet_map[frame_head] =
                int(&recv_cache_[offset] - frame_head + 1);
            frame_head = nullptr;
          } else {
            GatUtil::DbgPrintBinary(
                recv_cache_, cache_size_, "drop the headless frame: ");
          }
        }
      }
    }
  }

  if (gat_packet_map.size() == 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "recv invalid overlay package, can't parse any valid frame.";
    return;
  }

    {
        std::unique_lock <std::mutex> guard(gat_packet_mutex_);
        for (auto &item: gat_packet_map)
        {
            uint8_t* frame_addr = item.first;
            int frame_len = item.second;
            // 打印帧的起始地址和长度
            TRAFFIC_LIFGT_SERVICE_LOG_ERROR  << "Frame address: " << static_cast<void*>(frame_addr) << ", Frame length: " << frame_len << std::endl;
            gat_packet_.push(std::string((char *) item.first, (int) item.second));
            std::stringstream ss;
            for (int i = 0; i < frame_len; i++)
            {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(frame_addr[i]);
            }
            TRAFFIC_LIFGT_SERVICE_LOG_ERROR << ss.str();
            std::pair<int, std::string> src_data_pair;
            uint8_t *packet_addr = (uint8_t *)(item.first);
            uint16_t object_logo = BytesToU16(&packet_addr[26]);
            int msg_type;
            switch (object_logo)
            {
                case 0x0103:  //信号机灯色状态
                    msg_type = 0x0103;
                    break;
                case 0x0301:  //当前信号方案色步信息
                    msg_type = 0x0301;
                    break;
                case 0x0302: //下一周期信号方案色步信息
                    msg_type = 0x0302;
                    break;
                case 0x0101:           //信号灯运行状态
                    msg_type = 0x0101;
                    break;
                case 0x0102:            //信号灯控制方式
                    msg_type = 0x0102;
                    break;
                default:
                    msg_type = 0;
                    break;
            }
            src_data_pair.first = msg_type;
            src_data_pair.second = ss.str();
            gat_packet_src_.push(src_data_pair);
        }
    }

  for (auto &item : gat_packet_map) {
    this->ParseOnePacket(item.first, item.second);
  }

  memcpy(recv_cache_, recv_cache_ + shear_len, cache_size_ - shear_len);
  cache_size_ -= shear_len;
}

size_t GatParser::MakePacketQueryColorState(
    uint8_t *packet_buf, int packet_len) {
  if (packet_len < 35) {
    return 0;
  }
  return this->MakeQueryPacket(packet_buf, 0x0103);
}

size_t GatParser::MakePacketQueryCurrPlanStep(
    uint8_t *packet_buf, int packet_len) {
  if (packet_len < 35) {
    return 0;
  }
  return this->MakeQueryPacket(packet_buf, 0x0301);
}

uint16_t GatParser::BytesToU16(uint8_t *data) {
  return uint16_t(data[0] | (data[1] << 8));
}

uint32_t GatParser::BytesToU32(uint8_t *data) {
  return uint32_t(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

uint16_t GatParser::GatCrc16(uint8_t *data, uint32_t data_len) {
  uint16_t crc_in   = 0xFFFF;
  uint16_t crc_poly = 0x8005;
  uint8_t one_char  = 0;

  while (data_len--) {
    one_char = *(data++);
    crc_in ^= (one_char << 8);

    for (int i = 0; i < 8; i++) {
      if (crc_in & 0x8000) {
        crc_in = (crc_in << 1) ^ crc_poly;
      } else {
        crc_in = crc_in << 1;
      }
    }
  }

  return crc_in;
}

int GatParser::RemoveEscapeChar(uint8_t *frame_buf, int frame_len) {
  for (int i = 1; i < (frame_len - 1); i++) {
    if ((frame_buf[i] == 0xDB) && (frame_buf[i + 1] == 0xDC)) {
      frame_buf[i] = 0xC0;
      memmove(frame_buf + i + 1, frame_buf + i + 2, frame_len - i - 2);
      frame_len--;
    } else if ((frame_buf[i] == 0xDB) && (frame_buf[i + 1] == 0xDD)) {
      frame_buf[i] = 0xDB;
      memmove(frame_buf + i + 1, frame_buf + i + 2, frame_len - i - 2);
      frame_len--;
    }
  }

  return frame_len;
}

size_t GatParser::MakeQueryPacket(uint8_t *frame_addr, uint16_t obj_flag) {
  size_t offset = 0;

  // 1. 包头1B
  frame_addr[offset++] = 0xC0;
  // 2 链路码2B: 除信号机以外的其他路侧交通管控设备
  frame_addr[offset++] = 0x04;
  frame_addr[offset++] = 0x00;
  // 3.1 发送方标识 - 行政区代码3B
  frame_addr[offset++] = (uint8_t)(kAdministrativeCode & 0xFF);
  frame_addr[offset++] = (uint8_t)((kAdministrativeCode >> 8) & 0xFF);
  frame_addr[offset++] = (uint8_t)((kAdministrativeCode >> 16) & 0xFF);
  // 3.2 发送方标识 - 类型2B: 除信号机以外的其他路侧交通管控设备
  frame_addr[offset++] = 0x00;
  frame_addr[offset++] = 0x20;
  // 3.3 发送方标识 - 编号2B: 暂定为路口号
  frame_addr[offset++] = 0x01;  //(uint8_t)(FLAGS_rsu_intersection_id & 0xFF);
  frame_addr[offset++] =
      0x00;  // (uint8_t)((FLAGS_rsu_intersection_id >> 8) & 0xFF);
  // 4.1 接收方标识 - 行政区代码3B
  frame_addr[offset++] = (uint8_t)(kAdministrativeCode & 0xFF);
  frame_addr[offset++] = (uint8_t)((kAdministrativeCode >> 8) & 0xFF);
  frame_addr[offset++] = (uint8_t)((kAdministrativeCode >> 16) & 0xFF);
  // 4.2 发送方标识 - 类型2B: 信号机
  frame_addr[offset++] = 0x00;
  frame_addr[offset++] = 0x10;
  // 4.3 发送方标识 - 编号2B: 广播
  frame_addr[offset++] = 0xFF;
  frame_addr[offset++] = 0xFF;
  // 5. 时间戳6B：单位秒
  const uint32_t cur_time_stamp =
      uint32_t(GatUtil::GetCurTimestampMsec() / 1000);
  frame_addr[offset++] = (uint8_t)(cur_time_stamp & 0xFF);
  frame_addr[offset++] = (uint8_t)((cur_time_stamp >> 8) & 0xFF);
  frame_addr[offset++] = (uint8_t)((cur_time_stamp >> 16) & 0xFF);
  frame_addr[offset++] = (uint8_t)((cur_time_stamp >> 24) & 0xFF);
  frame_addr[offset++] = 0;
  frame_addr[offset++] = 0;
  // 6. 生存时间1B
  frame_addr[offset++] = 0x3C;
  // 7. 协议版本1B
  frame_addr[offset++] = 0x10;
  // 8. 操作类型1B : 查询请求
  frame_addr[offset++] = 0x80;
  // 9. 对象标识2B :
  frame_addr[offset++] = (uint8_t)(obj_flag & 0xFF);
  frame_addr[offset++] = (uint8_t)((obj_flag >> 8) & 0xFF);
  // 10. 签名标识1B
  frame_addr[offset++] = 0;
  // 11. 保留未使用3B
  frame_addr[offset++] = 0;
  frame_addr[offset++] = 0;
  frame_addr[offset++] = 0;
  // 12. 消息内容：查询无消息内容
  // 13. 校验码2B：CRC16
  const uint16_t crc16 = GatCrc16(frame_addr + 1, offset - 1);
  frame_addr[offset++] = (uint8_t)(crc16 & 0xFF);
  frame_addr[offset++] = (uint8_t)((crc16 >> 8) & 0xFF);
  // 14. 包尾1B
  frame_addr[offset++] = 0xC0;

  // 数据转义处理
  for (size_t i = 1; i < (offset - 1); i++) {
    if (frame_addr[i] == 0xC0) {
      memmove(frame_addr + i + 2, frame_addr + i + 1, offset - i - 1);
      offset++;

      frame_addr[i]     = 0xDB;
      frame_addr[i + 1] = 0xDC;
    } else if (frame_addr[i] == 0xDB) {
      memmove(frame_addr + i + 2, frame_addr + i + 1, offset - i - 1);
      offset++;

      frame_addr[i]     = 0xDB;
      frame_addr[i + 1] = 0xDD;
    }
  }

  return offset;
}

bool GatParser::ParseColorState(uint8_t *frame_addr, int packet_len) {
  if (packet_len < 48) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "curr period error, frame data len invalid.";
    return false;
  }

  uint8_t entrance_cnt = frame_addr[44];
  if (entrance_cnt == 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "parse phase color error, entrance cnt = "
              << unsigned(entrance_cnt);
    return false;
  }

  int offset              = 45;
  uint8_t light_cnt       = 0;
  uint8_t light_type      = 0;
  uint8_t light_color     = 0;
  uint8_t light_countdown = 0;
  uint8_t light_id        = 0;

  uint16_t tmp_entry_direction = 0; // 车道进口方向
  uint8_t phase_id_high = 0;  // 根据方向计算出高位ID

  std::map<uint8_t, GatLightState> new_light_state_map;

  for (uint8_t i = 0; i < entrance_cnt; i++) {
    if ((offset + 3) >= packet_len) {
      TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "curr period error, frame data len illegal.";
      return false;
    }

    tmp_entry_direction = BytesToU16(&frame_addr[offset]);
    phase_id_high = static_cast<uint8_t>(std::floor(tmp_entry_direction / 22.5));

    light_cnt = frame_addr[offset + 2];
    offset += 3;

    for (uint8_t j = 0; j < light_cnt; j++) {
      if ((offset + 4) >= packet_len) {
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "curr period error, frame data len illegal.";
        return false;
      }
      light_id    = int(frame_addr[offset]);
      light_type  = frame_addr[offset + 1];
      light_color = frame_addr[offset + 2];
      light_color = this->CovertGatLightColor(light_color);
      if (light_color == 0) {
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "light state invalid, light_id=" << unsigned(light_id)
                  << " color is zero.";
      }
      light_countdown                   = frame_addr[offset + 3];
      GatLightState tmp_one_light_state = {
        color : light_color,
        type : light_type,
        countdown : light_countdown,
        timestamp : GatUtil::GetCurTimestampMsec(),
        entry_direction : tmp_entry_direction
      };

      // 根据<<T_ITS 0117-2022合作式智能运输系统 RSU与中心子系统间数据接口规范>> 附录E
      // 对phaseId进行赋值, 高4bit为进口道方向, 低4位为灯组类型
      uint8_t phaseId = (phase_id_high << 4) | (light_type & 0x0F);
      TRAFFIC_LIFGT_SERVICE_LOG_INFO << "direction: " << tmp_entry_direction << "high: " << int(phase_id_high);
      TRAFFIC_LIFGT_SERVICE_LOG_INFO << "phaseId: " << int(phaseId) << ", light_type: " << int(light_type);
      new_light_state_map[phaseId] = std::move(tmp_one_light_state);
      offset += 4;
    }
  }

  if (new_light_state_map.size() > 0) {
    this->ResetAllColorState(new_light_state_map);
  }
  sendSourceDataToCloud(frame_addr, packet_len);
  return true;
}

bool GatParser::ParseCurrPlanStep(uint8_t *packet_addr, int packet_len) {
  GatUtil::DbgPrintBinary(packet_addr, packet_len, "ParseCurrPlanStep: ");
  if (packet_len < 49) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "curr period error, frame data len invalid.";
    return false;
  }

  uint8_t entrance_cnt = packet_addr[45];
  if (entrance_cnt == 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "curr period error, frame data no entrance.";
    return false;
  }

  int offset             = 46;
  uint8_t light_cnt      = 0;
  uint8_t light_id       = 0;
  uint8_t light_type     = 0;
  uint8_t light_color    = 0;
  uint16_t light_period  = 0;
  uint8_t light_step_cnt = 0;
  std::map<uint8_t, std::vector<GatLightStep>> tmp_all_period;

  for (uint8_t i = 0; i < entrance_cnt; i++) {
    if ((offset + 3) >= packet_len) {
      TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "curr period error, frame data len illegal.";
      return false;
    }
    light_cnt = packet_addr[offset + 2];
    offset += 3;

    for (uint8_t j = 0; j < light_cnt; j++) {
      if ((offset + 3) >= packet_len) {
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "curr period error, frame data len illegal.";
        return false;
      }
      light_id       = packet_addr[offset];
      light_type     = packet_addr[offset + 1];
      light_step_cnt = packet_addr[offset + 2];
      offset += 3;
      for (uint8_t k = 0; k < light_step_cnt; k++) {
        if ((offset + 3) >= packet_len) {
          TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "curr period error, frame data len illegal.";
          return false;
        }
        light_color = packet_addr[offset];
        light_color = this->CovertGatLightColor(light_color);
        if (light_color == 0) {
          TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "light state invalid, light_id=" << unsigned(light_id)
                    << " color is zero.";
        }
        light_period =
            uint16_t(packet_addr[offset + 1] | (packet_addr[offset + 2] << 8));
        offset += 3;

        GatLightStep tmp_pmd = {
          color : light_color,
          type : light_type,
          duration : light_period
        };
        if (tmp_all_period.count(light_id) > 0) {
          tmp_all_period[light_id].push_back(tmp_pmd);
        } else {
          std::vector<GatLightStep> tmp_v_pmd;
          tmp_v_pmd.push_back(tmp_pmd);
          tmp_all_period[light_id] = tmp_v_pmd;
        }
      }
    }
  }

  if (tmp_all_period.size() > 0) {
    this->ResetAllCurrPlanStep(tmp_all_period);
  }
  sendSourceDataToCloud(packet_addr, packet_len);
  return true;
}

bool GatParser::ParseRunState(uint8_t *packet_addr, int packet_len) {
  if (packet_len < 52) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "run state error, packet data len invalid, len="
              << unsigned(packet_len);
    return false;
  }

  uint32_t utc_timestamp_sec = BytesToU32(&packet_addr[40]);
  running_state_             = packet_addr[44];

  std::string sta_str = "error";
  switch (running_state_) {
    case 0:
      sta_str = "invalid";
      break;
    case 1:
      sta_str = "normal";
      break;
    case 2:
      sta_str = "fault";
      break;
    case 3:
      sta_str = "other";
      break;
    default:
      break;
  }

  TRAFFIC_LIFGT_SERVICE_LOG_WARN << "GAT-RUN_STATE: timestamp=" << utc_timestamp_sec
           << "state=" << sta_str;

  return true;
}

bool GatParser::ParseControlMode(uint8_t *packet_addr, int packet_len) {
  if (packet_len < 52) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "run state error, packet data len invalid, len="
              << unsigned(packet_len);
    return false;
  }

  uint32_t utc_timestamp_sec = BytesToU32(&packet_addr[40]);
  control_mode_              = packet_addr[44];

  std::string mode_str = "error";
  switch (control_mode_) {
    case 1:
      mode_str = "yellow-flash";
      break;
    case 2:
      mode_str = "multi-period";
      break;
    case 3:
      mode_str = "manual";
      break;
    case 4:
      mode_str = "induction";
      break;
    case 5:
      mode_str = "no-cable-coordination";
      break;
    case 6:
      mode_str = "single-spot-optimization";
      break;
    case 7:
      mode_str = "Bus-signal-priority";
      break;
    case 8:
      mode_str = "Emergency-priority";
      break;
    case 9:
      mode_str = "other";
      break;
    default:
      break;
  }

  TRAFFIC_LIFGT_SERVICE_LOG_WARN << "GAT-CONTROL_MODE: timestamp=" << utc_timestamp_sec
           << "ControlMode=" << mode_str;
  return true;
}

void GatParser::ParseOnePacket(uint8_t *packet_addr, int packet_len) {
  monitor_->IncGatPacketCounter();

  // TODO: for debug
  GatUtil::DbgPrintBinary(
      packet_addr, packet_len, std::string("parse gat frame : "));

  if (packet_len <= 35 || packet_len > kPacketBuffCapacity) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "parse packet error, packet length less than 35.";
    monitor_->IncGatBadPacketCounter();
    return;
  }

  memcpy(packet_buff_, packet_addr, packet_len);

  packet_len =
      this->RemoveEscapeChar(packet_buff_, packet_len);  // 转义数据处理

  // 校验码检验
  uint16_t crc16_a = BytesToU16(&packet_buff_[packet_len - 3]);
  uint16_t crc16_b = GatCrc16(packet_buff_ + 1, packet_len - 4);
  if (crc16_a != crc16_b) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "parse packet error, crc16 invalid.";
    monitor_->IncGatBadPacketCounter();
    return;
  }

  uint16_t object_logo = BytesToU16(&packet_buff_[26]);

  switch (object_logo) {
    case 0x0103:
      this->ParseColorState(packet_buff_, packet_len);
      break;
    case 0x0301:
      this->ParseCurrPlanStep(packet_buff_, packet_len);
      break;
    case 0x0302:
      TRAFFIC_LIFGT_SERVICE_LOG_WARN << "ParseNextPeriod";
      break;
    case 0x0101:
      this->ParseRunState(packet_buff_, packet_len);
      break;
    case 0x0102:
      this->ParseControlMode(packet_buff_, packet_len);
      break;
    default:
      TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "invalid object-log gat frame : " << unsigned(object_logo);
      return;
  }
}

void GatParser::ResetAllColorState(
    std::map<uint8_t, GatLightState> &new_color_state) {
  monitor_->FlushColorState(new_color_state);

  bool is_changed = false;
  std::map<uint8_t, GatLightState> tmp_all_color_state;
  this->GetAllColorState(tmp_all_color_state);

  if (new_color_state.size() != tmp_all_color_state.size()) {
    is_changed = true;
  } else {
    for (auto &kv : new_color_state) {
      GatLightState &one_new_state = kv.second;
      GatLightState &one_old_state = tmp_all_color_state[kv.first];
      if ((one_old_state.color != one_new_state.color) ||
          (one_old_state.type != one_new_state.type) ||
          (one_old_state.countdown != one_new_state.countdown)) {
        is_changed = true;
        break;
      }
    }
  }

  if (is_changed) {
    this->SetAllColorState(new_color_state);
    this->JudgeProcessSeconds();  // 当前灯色状态有改变时，重新判断周期行进度
  }
}

void GatParser::ResetAllCurrPlanStep(
    std::map<uint8_t, std::vector<GatLightStep>> &new_curr_plan_step) {
  // 将闪灯转为亮灯
  for (auto &kv : new_curr_plan_step) {
    for (auto &one_period : kv.second) {
      if (one_period.color > 10) {
        one_period.color = one_period.color - 10;
      }
    }
  }

  // 相邻同色灯时长结合
  std::map<uint8_t, std::vector<GatLightStep>> cur_new_cur_plan_step;
  for (auto &kv : new_curr_plan_step) {
    for (auto &item : kv.second) {
      std::vector<GatLightStep> &the_period = cur_new_cur_plan_step[kv.first];
      if (!the_period.empty()) {
        if (the_period[the_period.size() - 1].color != item.color) {
          the_period.push_back(item);
        } else {
          the_period[the_period.size() - 1].duration += item.duration;
        }
      } else {
        the_period.push_back(item);
      }
    }
  }

  monitor_->FlushCurrPlanStep(cur_new_cur_plan_step);

  bool is_changed = false;
  std::map<uint8_t, std::vector<GatLightStep>> tmp_all_light_period;
  this->GetAllCurrPlanStep(tmp_all_light_period);

  if (cur_new_cur_plan_step.size() != tmp_all_light_period.size()) {
    is_changed = true;
  } else {
    for (auto &kv : cur_new_cur_plan_step) {
      std::vector<GatLightStep> &one_new_period = kv.second;
      std::vector<GatLightStep> &one_old_period =
          tmp_all_light_period[kv.first];
      if (one_new_period.size() != one_old_period.size()) {
        is_changed = true;
        break;
      }

      const int max_i(one_new_period.size());
      for (int i = 0; i < max_i; i++) {
        if ((one_new_period[i].color != one_old_period[i].color) ||
            (one_new_period[i].duration != one_old_period[i].duration)) {
          is_changed = true;
          break;
        }
      }

      if (is_changed) {
        break;
      }
    }
  }

  if (is_changed) {
    this->SetAllCurrPlanStep(cur_new_cur_plan_step);
    // 更新总周期时长
    uint16_t tmp_total_period = 0;
    for (auto &kv : cur_new_cur_plan_step) {
      for (auto &item : kv.second) {
        tmp_total_period += item.duration;
      }
      break;
    }
    total_period_ = tmp_total_period;

    this->JudgeProcessSeconds();  // 周期有改变时，重新判断周期行进度
  }

  return;
}

/**
 * @brief GAT1743灯色转为内部定义的灯色值
 *
 * @param gat_light_color
 *    1．按比特位定义灯组发光单元
 *        1）当灯组类型为1~12时，
 *            Bit1~Bit0用于表示红色发光单元，
 *            Bit3~Bit2用于表示黄色发光单元，
 *            Bit5~Bit4用于表示绿色发光单元，
 *            Bit7~Bit6保留
 *        2）当灯组类型为13~15时，
 *            Bit1~Bit0用于表示禁止通行信号发光单元，
 *            Bit3~Bit2用于表示过渡信号发光单元，
 *            Bit5~Bit4用于表示通行信号发光单元，
 *            Bit7~Bit6保留
 *    2．具体取值
 *        0：无灯
 *        1：灭灯
 *        2：亮灯
 *        3：闪烁
 * @return uint8_t (0-关灯 1-红 2-黄 3-绿  11-红闪 12-黄闪 13-绿闪)
 */
uint8_t GatParser::CovertGatLightColor(uint8_t gat_light_color) {
  uint8_t std_light_color = 0;
  if (gat_light_color & 0x2) {  // red
    if ((gat_light_color & 0x3) == 0x2) {
      std_light_color = 1;
    } else {
      std_light_color = 11;
    }
  } else if ((gat_light_color >> 2) & 0x2) {  // yellow
    if (((gat_light_color >> 2) & 0x3) == 0x2) {
      std_light_color = 2;
    } else {
      std_light_color = 12;
    }
  } else if ((gat_light_color >> 4) & 0x2) {  // green
    if (((gat_light_color >> 4) & 0x3) == 0x2) {
      std_light_color = 3;
    } else {
      std_light_color = 13;
    }
  }

  return std_light_color;
}

// 根据当前方案周期和当前灯色状态判断当前时间处于周期内的第几秒
void GatParser::JudgeProcessSeconds() {
  std::map<uint8_t, GatLightState> cur_light_state;
  this->GetAllColorState(cur_light_state);
  bool is_normal = false;  // 判断是否倒计时正常(如进入手控模式后倒计时全部为0)

  for (auto &kv : cur_light_state) {
    if (kv.second.countdown != 0) {
      is_normal = true;
    }
    if (kv.second.color > 10) {  // 将闪灯转为亮灯
      kv.second.color = kv.second.color - 10;
    }
  }

  if (!is_normal) {
    progress_seconds_ = 0;  // 异常状态，行进秒数清零
    return;
  }

  // 判定周期内进行秒数
  std::map<uint8_t, std::vector<GatLightStep>> tmp_all_curr_plan_step;
  this->GetAllCurrPlanStep(tmp_all_curr_plan_step);
  for (auto &kv_state : cur_light_state) {
    if (kv_state.second.countdown == 0) {
      continue;  // 无效倒计时，忽略之
    }
    if (tmp_all_curr_plan_step.count(kv_state.first) == 0) {
      continue;  // 该灯组周期不存在，忽略之
    }

    // 试图找到这样一个灯：它一个周期内只有一个当前灯色的步色
    std::vector<GatLightStep> &one_light_steps =
        tmp_all_curr_plan_step[kv_state.first];
    int match_color_times = 0;
    int index_max         = one_light_steps.size();
    for (int i = 0; i < index_max; i++) {
      if ((one_light_steps[i].color == kv_state.second.color) &&
          !((i == (index_max - 1)) &&
            (one_light_steps[i].color == one_light_steps[0].color))) {
        match_color_times += 1;
      }
    }

    if (match_color_times == 1) {  // 已找只包含一种步色的灯组周期
      uint16_t tmp_seconds = 0;
      if ((one_light_steps[index_max - 1].color == one_light_steps[0].color) &&
          (kv_state.second.color == one_light_steps[0].color)) {
        // 首尾同色，并且当前灯色和首尾灯色一致，判断倒计时处于首步色还是尾步色
        if (kv_state.second.countdown > one_light_steps[0].duration) {
          // 如果倒计时大于首步色周期时长，则说明处于尾步色
          tmp_seconds = total_period_ - (kv_state.second.countdown -
                                         one_light_steps[0].duration);
        } else {
          // 如果倒计时小于等于首步色周期时长，则说明处于首步色
          tmp_seconds = one_light_steps[0].duration - kv_state.second.countdown;
        }
      } else {
        for (auto &one_step_period : tmp_all_curr_plan_step[kv_state.first]) {
          tmp_seconds += one_step_period.duration;
          if (one_step_period.color ==
              kv_state.second.color) {  // 定位到当前灯色所在的步色
            tmp_seconds -= kv_state.second.countdown;
            break;
          }
        }
      }
      progress_seconds_ = tmp_seconds + 1;  // 周期内行进的秒数从1开始
      break;
    }
  }
}

void GatParser::GetAllColorState(
    std::map<uint8_t, GatLightState> &color_state) {
  std::unique_lock<std::mutex> guard(all_color_state_mutex_);
  color_state.clear();
  color_state = all_color_state_;
}

void GatParser::SetAllColorState(
    std::map<uint8_t, GatLightState> &color_state) {
  std::unique_lock<std::mutex> guard(all_color_state_mutex_);
  all_color_state_.clear();
  all_color_state_ = color_state;
}

size_t GatParser::GetAllColorStateSize() {
  std::unique_lock<std::mutex> guard(all_color_state_mutex_);
  return all_color_state_.size();
}

void GatParser::GetAllCurrPlanStep(
    std::map<uint8_t, std::vector<GatLightStep>> &curr_plan_step) {
  std::unique_lock<std::mutex> guard(all_curr_plan_step_mutex_);
  curr_plan_step.clear();
  curr_plan_step = all_curr_plan_step_;
}

void GatParser::SetAllCurrPlanStep(
    std::map<uint8_t, std::vector<GatLightStep>> &curr_plan_step) {
  std::unique_lock<std::mutex> guard(all_curr_plan_step_mutex_);
  all_curr_plan_step_.clear();
  all_curr_plan_step_ = curr_plan_step;
}

size_t GatParser::GetAllCurrPlanStepSize() {
  std::unique_lock<std::mutex> guard(all_curr_plan_step_mutex_);
  return all_curr_plan_step_.size();
}

bool GatParser::IsDataReady() {
  if (this->GetAllColorStateSize() == 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_WARN << "color still not received.";
    return false;
  }

  if (this->GetAllCurrPlanStepSize() == 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_WARN << "period still not received.";
    // return false;
  }

  if (monitor_->IsDataExpired()) {
    TRAFFIC_LIFGT_SERVICE_LOG_WARN << "light state lose efficacy.";
    return false;
  }

  return true;
}

LightState GatParser::PbCovertLightColor(uint8_t light_color) {
  // light_color:  0-关灯 1-红 2-黄 3-绿  11-红闪 12-黄闪 13-绿闪
  switch (light_color) {
    case 0:
      return os::v2x::device::DARK;
    case 1:
      return os::v2x::device::RED;
    case 11:
      return os::v2x::device::FLASHING_RED;
    case 2:
      return os::v2x::device::YELLOW;
    case 12:
      return os::v2x::device::FLASHING_YELLOW;
    case 3:
      return os::v2x::device::GREEN;
    case 13:
      return os::v2x::device::FLASHING_GREEN;
    default:
      return os::v2x::device::UNAVAILABLE;
  }
};

LightType GatParser::PbCovertLightType(uint8_t light_type) {
  const static os::v2x::device::LightType light_type_table[16] = {
      os::v2x::device::LIGHT_TYPE_UNKNOWN,
      os::v2x::device::STRAIGHT_DIRECTION,
      os::v2x::device::LEFT_DIRECTION,
      os::v2x::device::RIGHT_DIRECTION,
      os::v2x::device::MOTOR_VEHICLE,
      os::v2x::device::NON_MOTOR_VEHICLE_LEFT,
      os::v2x::device::NON_MOTOR_VEHICLE_RIGHT,
      os::v2x::device::NON_MOTOR_VEHICLE,
      os::v2x::device::CROSSWALK,
      os::v2x::device::TURN_DIRECTION,
      os::v2x::device::LANE,
      os::v2x::device::CROSSING,
      os::v2x::device::FLASHING_WARNING,
      os::v2x::device::TRAMCAR_STRAIGHT,
      os::v2x::device::TRAMCAR_LEFT,
      os::v2x::device::TRAMCAR_RIGHT};

  LightType tmp_light_type;
  if (light_type > 15) {
    tmp_light_type = os::v2x::device::LIGHT_TYPE_UNKNOWN;
  } else {
    tmp_light_type = light_type_table[light_type];
  }
  return tmp_light_type;
};

void GatParser::sendSourceDataToCloud(uint8_t *data, const int len)
{
	if(socket_fd_ > 0)
	{
		for(int i = 0; i < outlist.size(); i++)
		{
			struct sockaddr_in peer_addr_;
			peer_addr_.sin_family      = AF_INET;
			peer_addr_.sin_addr.s_addr = inet_addr(outlist[i].peerip.c_str());
			peer_addr_.sin_port        = htons(outlist[i].peerport);
			sendto(socket_fd_, data, len, 0, (struct sockaddr *)&peer_addr_, sizeof(peer_addr_));
		}
	}

	struct sockaddr_in peer_addr_;
	peer_addr_.sin_addr.s_addr = inet_addr("127.0.0.1");
	peer_addr_.sin_port        = htons(50500);
	sendto(socket_fd_, data, len, 0, (struct sockaddr *)&peer_addr_, sizeof(peer_addr_));
}


}  // namespace device
}  // namespace v2x
}  // namespace os
