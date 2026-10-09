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

#include "internal_protocol.h"

#include <sys/time.h>

#include <iostream>

#include "base/common/log.h"

namespace os {
namespace v2x {
namespace device {

std::string InternalProtocol::Encode(const RSUDataConstPtr& data) {
  if (data == nullptr) {
    return "";
  }
  uint8_t type = 0;
  switch (data->type()) {
    case os::v2x::device::RSU_BSM:
      type = MessageType::MSG_BSM;
      break;
    case os::v2x::device::RSU_RSM:
      type = MessageType::MSG_RSM;
      break;
    case os::v2x::device::RSU_MAP:
      type = MessageType::MSG_MAP;
      break;
    case os::v2x::device::RSU_SPAT:
      type = MessageType::MSG_SPAT;
      break;
    case os::v2x::device::RSU_RSI:
      type = MessageType::MSG_RSI;
      break;
    case os::v2x::device::RSU_PERCEPTION:
      type = MessageType::MSG_PERCEPTION;
      break;
    default:
      RSU_SERVICE_LOG_WARN << "not support msg type: " << unsigned(data->type());
      return "";
  }

  const char* spdu_buf = data->data().c_str();
  int spdu_len         = data->data().size();

  std::string pack;
  pack.assign(spdu_len + 8 + 3, '\0');

  pack[0] = 0xfa;  // 包头
  pack[1] = 0xfb;  // 包头
  pack[2] = 0x01;  // 版本号
  pack[3] = 0x01;  // 版本号
  pack[4] = 0x22;  // 发送方标识
  pack[5] = type;  // type
  pack[6] = spdu_len / 256;
  pack[7] = spdu_len % 256;
  memcpy(&pack[8], spdu_buf, spdu_len);  // ASN数据流体
  pack[8 + spdu_len] = 0;                // 校验码
  for (int i = 2; i < 8 + spdu_len; i++) {
    pack[8 + spdu_len] ^= pack[i];
  }
  pack[9 + spdu_len]  = 0xec;  // 包尾
  pack[10 + spdu_len] = 0xed;  // 包尾
  return pack;
}

bool InternalProtocol::Decode(
    const char* pack, std::size_t len, RSUDataPtr& data) {
  if (pack == nullptr || len < 11 || len > 1900 || data == nullptr) {
    RSU_SERVICE_LOG_ERROR << "intput param error";
    return false;
  }
  /// 检查包头包尾是指定字符
  if (uint8_t(pack[0]) != 0xfa || uint8_t(pack[1]) != 0xfb) {
    RSU_SERVICE_LOG_ERROR << "frame header error";
    return false;
  }
  if (uint8_t(pack[len - 1]) != 0xed || uint8_t(pack[len - 2]) != 0xec) {
    RSU_SERVICE_LOG_ERROR << "frame tail error";
    return false;
  }
  /// 检查实际长度与标识长度一致
  uint16_t spdu_len = uint8_t(pack[6]) << 8 | uint8_t(pack[7]);
  if (len - 11 != spdu_len) {
    RSU_SERVICE_LOG_ERROR << "frame length error";
    return false;
  }
  /// 检查校验码一致
  uint8_t check_code = 0x00;
  for (int i = 2; i < 8 + spdu_len; i++) {
    check_code ^= uint8_t(pack[i]);
  }
  if (uint8_t(pack[8 + spdu_len]) != check_code) {
    RSU_SERVICE_LOG_ERROR << "frame check_code error";
    return false;
  }

  uint8_t type = pack[5];
  os::v2x::device::RSUDataType data_type;
  switch (type) {
    case MessageType::MSG_BSM:
      data_type = os::v2x::device::RSU_BSM;
      break;
    case MessageType::MSG_RSM:
      data_type = os::v2x::device::RSU_RSM;
      break;
    case MessageType::MSG_MAP:
      data_type = os::v2x::device::RSU_MAP;
      break;
    case MessageType::MSG_SPAT:
      data_type = os::v2x::device::RSU_SPAT;
      break;
    case MessageType::MSG_RSI:
      data_type = os::v2x::device::RSU_RSI;
      break;
    case MessageType::MSG_PERCEPTION:
      data_type = os::v2x::device::RSU_PERCEPTION;
      break;
    default:
      RSU_SERVICE_LOG_WARN << "Received the unknown message type: " << unsigned(pack[5]);
      data_type = os::v2x::device::RSU_UNKNOWN;
      break;
  }

  struct timeval tv;
  gettimeofday(&tv, NULL);
  data->set_time_stamp(
      static_cast<uint64_t>(tv.tv_sec * 1000 + tv.tv_usec / 1000));
  data->set_data((const void*)(pack + 8), (size_t)spdu_len);
  data->set_type(data_type);

  data->set_sequence_num(++sequence_num % 128);
  data->set_version(os::v2x::device::YDT_3709_2020);
  return true;
}

}  // namespace device
}  // namespace v2x
}  // namespace os
