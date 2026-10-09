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

#include <stdio.h>
#include <unistd.h>

#include <cassert>
#include <iostream>
#include <string>

#include "gtest/gtest.h"

#include "base/common/log.h"
#include "base/device_connect/traffic_light/device_factory.h"
#include "middleware/device_service/modules/traffic_light/gat1743/gat_communication.h"
#include "middleware/device_service/modules/traffic_light/gat1743/gat_monitor.h"
#include "middleware/device_service/modules/traffic_light/gat1743/gat_parser.h"
#include "middleware/device_service/modules/traffic_light/gat1743/gat_util.h"

namespace os {
namespace v2x {
namespace device {

static std::stringstream g_traffic_light_buf;
void proc_traffic_light_data(const TrafficLightDataType &traffic_data) {
  std::cout << traffic_data->sequence_num();
}

void output_traffic_light_data(
    const os::v2x::device::TrafficLightDataType &traffic_data) {
  TRAFFIC_LIFGT_SERVICE_LOG_WARN << traffic_data->DebugString();
}

class TrafficLightTest : public ::testing::Test {
 public:
  TrafficLightTest()
      : device_(nullptr)
      , sbuf_(nullptr) {}
  virtual ~TrafficLightTest() {}
  void SetUp() override {
    sbuf_ = std::cout.rdbuf();
    std::cout.rdbuf(g_traffic_light_buf.rdbuf());
  }
  void TearDown() override {
    std::cout.rdbuf(sbuf_);
    sbuf_ = nullptr;
  }

 protected:
  std::shared_ptr<TrafficLightDevice> device_;
  std::streambuf *sbuf_;
};

TEST_F(TrafficLightTest, test_all_interface) {
  device_ = TrafficLightDeviceFactory::Instance().GetUnique(
      "dummy_traffic_light", proc_traffic_light_data);
  ASSERT_NE(device_, nullptr);
  ASSERT_TRUE(
      device_->Init("/airos/middleware/device_service/modules/traffic_light/ut/"
                    "traffic_light.cfg"));

  device_->Start();
  std::string expect{"01234"};
  std::string res = g_traffic_light_buf.str();
  EXPECT_EQ(expect, res);

  EXPECT_EQ(TrafficLightDeviceState::RUNNING, device_->GetState());
}

TEST_F(TrafficLightTest, test_init_failed) {
  device_ = TrafficLightDeviceFactory::Instance().GetUnique(
      "dummy_traffic_light", proc_traffic_light_data);
  ASSERT_NE(device_, nullptr);
  ASSERT_FALSE(
      device_->Init("/airos/base/device_connect/middleware/device_service/"
                    "modules/traffic_light/ut/"
                    "traffic_light.cfg-no-exists"));
}

TEST_F(TrafficLightTest, test_gat_traffic_light_init) {
  device_ = os::v2x::device::TrafficLightDeviceFactory::Instance().GetUnique(
      "gat_traffic_light", output_traffic_light_data);
  ASSERT_NE(device_, nullptr);
  ASSERT_TRUE(
      device_->Init("/airos/middleware/device_service/modules/traffic_light/ut/"
                    "traffic_light.cfg"));

  EXPECT_EQ(
      os::v2x::device::TrafficLightDeviceState::RUNNING, device_->GetState());
}

TEST_F(TrafficLightTest, test_gat_traffic_light_init_failed) {
  device_ = os::v2x::device::TrafficLightDeviceFactory::Instance().GetUnique(
      "gat_traffic_light", output_traffic_light_data);
  ASSERT_NE(device_, nullptr);
  ASSERT_FALSE(
      device_->Init("/airos/middleware/device_service/modules/traffic_light/ut/"
                    "traffic_light.cfg-no-exists"));
}

TEST_F(TrafficLightTest, test_gat_communication_udp) {
  {
    std::string remote_ip("127.0.0.1");
    uint16_t remote_port = 10023;
    std::string host_ip("0.0.0.1");
    uint16_t host_port = 10050;
    os::v2x::device::GatCommunication::ProtocolType protocol =
        os::v2x::device::GatCommunication::ProtocolType::UDP;
    os::v2x::device::GatCommunication comm;
    ASSERT_FALSE(
        comm.Init(remote_ip, remote_port, host_ip, host_port, protocol));
  }

  std::string remote_ip("127.0.0.1");
  uint16_t remote_port = 10023;
  std::string host_ip("127.0.0.1");
  uint16_t host_port = 10050;
  os::v2x::device::GatCommunication::ProtocolType protocol =
      os::v2x::device::GatCommunication::ProtocolType::UDP;

  os::v2x::device::GatCommunication comm;
  ASSERT_TRUE(comm.Init(remote_ip, remote_port, host_ip, host_port, protocol));

  ASSERT_TRUE(comm.Connect());

  uint8_t send_buf[4] = {0};
  ssize_t send_len    = comm.SendData(send_buf, 4);
  ASSERT_EQ(send_len, 4);

  ssize_t recv_len = comm.RecvDataWait(send_buf, 4);
  ASSERT_EQ(recv_len, 0);
}

TEST_F(TrafficLightTest, test_gat_communication_tcp) {
  {
    const std::string remote_ip("127.0.0.1");
    const uint16_t remote_port = 10023;
    const std::string host_ip("0.0.0.1");
    const uint16_t host_port = 10050;
    const os::v2x::device::GatCommunication::ProtocolType protocol =
        os::v2x::device::GatCommunication::ProtocolType::TCP;
    os::v2x::device::GatCommunication comm;
    ASSERT_FALSE(
        comm.Init(remote_ip, remote_port, host_ip, host_port, protocol));
  }

  const std::string remote_ip("127.0.0.1");
  const uint16_t remote_port = 10023;
  const std::string host_ip("127.0.0.1");
  const uint16_t host_port = 10050;
  const os::v2x::device::GatCommunication::ProtocolType protocol =
      os::v2x::device::GatCommunication::ProtocolType::TCP;
  os::v2x::device::GatCommunication comm;

  ASSERT_TRUE(comm.Init(remote_ip, remote_port, host_ip, host_port, protocol));

  ASSERT_TRUE(comm.Connect());

  uint8_t send_buf[4] = {0};
  ssize_t send_len    = comm.SendData(send_buf, 4);
  ASSERT_EQ(send_len, 4);

  ssize_t recv_len = comm.RecvDataWait(send_buf, 4);
  ASSERT_EQ(recv_len, 0);
}

TEST_F(TrafficLightTest, test_gat_util) {
  uint8_t bin_dat[4] = {0x11, 0xAF, 0xF1, 0};
  std::string str1   = os::v2x::device::GatUtil::Bin2Str(bin_dat, 4);
  ASSERT_STREQ(str1.c_str(), "11AFF100");

  os::v2x::device::GatUtil::DbgPrintBinary(bin_dat, 4, "test_util:");

  uint64_t t1 = os::v2x::device::GatUtil::GetCurTimestampMsec();
  usleep(10 * 1000);
  uint64_t t2 = os::v2x::device::GatUtil::GetCurTimestampMsec();
  ASSERT_GE((t2 - t1), 10);
}

TEST_F(TrafficLightTest, test_gat_monitor) {
  os::v2x::device::GatMonitor monitor;

  ASSERT_TRUE(monitor.Init());

  monitor.IncRecvFrameCounter();
  monitor.IncGatBadPacketCounter();

  ASSERT_EQ(monitor.GetColorStatePacketFreq(), 0.0);
  ASSERT_EQ(monitor.GetCurrPlanStepPacketFreq(), 0.0);

  ASSERT_TRUE(monitor.IsDataExpired());

  ASSERT_FALSE(monitor.IsRemoteAlive());
  monitor.IncGatPacketCounter();
  ASSERT_TRUE(monitor.IsRemoteAlive());

  uint64_t timestamp = os::v2x::device::GatUtil::GetCurTimestampMsec();

  std::map<uint8_t, os::v2x::device::GatLightState> new_lights_state = {
      {1, {
         color : 1,  // 0-关灯 1-红 2-黄 3-绿  11-红闪 12-黄闪 13-绿闪
         type : 2,
         countdown : 91,
         timestamp : timestamp
       }},
      {2, {color : 3, type : 1, countdown : 3, timestamp : timestamp}},
      {3, {color : 3, type : 1, countdown : 3, timestamp : timestamp}},
      {7, {color : 1, type : 9, countdown : 33, timestamp : timestamp}},
      {8, {color : 1, type : 8, countdown : 43, timestamp : timestamp}},
  };

  std::map<uint8_t, std::vector<os::v2x::device::GatLightStep>>
      new_lights_period = {
          {1,
           {{color : 3, type : 2, duration : 16},
            {color : 2, type : 2, duration : 4},
            {color : 1, type : 2, duration : 132}}},
          {2,
           {{color : 3, type : 1, duration : 64},
            {color : 2, type : 1, duration : 4},
            {color : 1, type : 1, duration : 84}}},
          {3,
           {{color : 3, type : 1, duration : 64},
            {color : 2, type : 1, duration : 4},
            {color : 1, type : 1, duration : 84}}},
          {7,
           {{color : 3, type : 9, duration : 74},
            {color : 2, type : 9, duration : 4},
            {color : 1, type : 9, duration : 74}}},
          {7,
           {{color : 1, type : 8, duration : 112},
            {color : 3, type : 8, duration : 40}}}};

  monitor.FlushCurrPlanStep(new_lights_period);
  monitor.FlushColorState(new_lights_state);

  ASSERT_FALSE(monitor.IsDataExpired());

  ASSERT_EQ(monitor.GetColorStatePacketFreq(), 1.0);
  monitor.ZeroColorStatePacketCounter();
  ASSERT_EQ(monitor.GetColorStatePacketFreq(), 0.0);

  monitor.FlushCurrPlanStep(new_lights_period);
  monitor.FlushCurrPlanStep(new_lights_period);
  monitor.FlushCurrPlanStep(new_lights_period);
  monitor.FlushCurrPlanStep(new_lights_period);
  ASSERT_EQ(monitor.GetCurrPlanStepPacketFreq(), 1.0);
  monitor.ZeroCurrPlanStepPacketCounter();
  ASSERT_EQ(monitor.GetCurrPlanStepPacketFreq(), 0.0);
}

int StringToBytes(const std::string &in_str, uint8_t *out_bytes) {
  int offset = 0;
  int inter  = 0;
  for (size_t i = 0; i < in_str.size(); i += 2) {
    sscanf(&in_str.at(i), "%02x", &inter);
    out_bytes[offset++] = uint8_t(inter);
  }

  return offset;
}

TEST_F(TrafficLightTest, test_gat_parser) {
  std::string light_state_packet =
      "c0040023ae010010ffff23ae010020ffff2ea46c630c013c10830301000000008700f984"
      "7245a3c5b7170000040000080d02162e0c02162e0e0125080f02162e1002162e1402162e"
      "1502162e1902162e5a0007010225170201161a03022517040225171102162e1602162e1a"
      "02162eb400070502162e060125080702162e0802162e1202162e1702162e1b02162e0e01"
      "060902160b0a01160b0b02160b1302162e1802162e1c02162e2235c0";
  std::string curr_period_packet =
      "c0040023ae010010ffff23ae010020ffff2aa46c6379013c10830103000000003402f984"
      "7245a3c5b71700001c040000080d0205163200251d00350300190300160f000c02051632"
      "00251d00350300190300160f000e0108250900350300190300162300251d003503001903"
      "00160f000f0205163200251d00350300190300160f00100205163200251d003503001903"
      "00160f00140205163200251d00350300190300160f00150205163200251d003503001903"
      "00160f00190205163200251d00350300190300160f005a00070102052518003503001903"
      "00162800251e00020108161e00250e003503001903001614002518003503001903000302"
      "05251800350300190300162800251e00040205251800350300190300162800251e001102"
      "05163200251d00350300190300160f00160205163200251d00350300190300160f001a02"
      "05163200251d00350300190300160f00b40007050205163200251d00350300190300160f"
      "00060108250900350300190300162300251d00350300190300160f00070205163200251d"
      "00350300190300160f00080205163200251d00350300190300160f00120205163200251d"
      "00350300190300160f00170205163200251d00350300190300160f001b0205163200251d"
      "00350300190300160f000e0106090205160f00251d003503001903001632000a0108160f"
      "00251d003503001903001623002509003503001903000b0205160f00251d003503001903"
      "00163200130205163200251d00350300190300160f00180205163200251d003503001903"
      "00160f001c0205163200251d00350300190300160f00ff88c0";

  std::shared_ptr<os::v2x::device::GatMonitor> monitor =
      std::make_shared<os::v2x::device::GatMonitor>();
  os::v2x::device::GatParser parser;
#if 0
  ASSERT_TRUE(parser.Init(monitor));

  uint8_t packet_buf[64] = {0};
  size_t ret_val         = 0;

  ret_val = parser.MakePacketQueryColorState(packet_buf, 30);
  ASSERT_EQ(ret_val, 0);
  ret_val = parser.MakePacketQueryColorState(packet_buf, 64);
  ASSERT_GE(ret_val, 35);

  ret_val = parser.MakePacketQueryCurrPlanStep(packet_buf, 30);
  ASSERT_EQ(ret_val, 0);
  ret_val = parser.MakePacketQueryCurrPlanStep(packet_buf, 64);
  ASSERT_GE(ret_val, 35);

  uint8_t tmp_buf[1024] = {0};
  int tmp_len           = 0;
  os::v2x::device::TrafficLightBaseData traffic_light_dat;

  ASSERT_FALSE(parser.GetTrafficLightData(traffic_light_dat));

  tmp_len = StringToBytes(light_state_packet, tmp_buf);
  parser.ProcessFrames(tmp_buf, tmp_len);

  ASSERT_TRUE(parser.GetTrafficLightData(traffic_light_dat));

  tmp_len = StringToBytes(curr_period_packet, tmp_buf);
  parser.ProcessFrames(tmp_buf, tmp_len);

  ASSERT_TRUE(parser.GetTrafficLightData(traffic_light_dat));
  ASSERT_GE(
      os::v2x::device::GatUtil::GetCurTimestampMsec(),
      traffic_light_dat.time_stamp());
  ASSERT_EQ(traffic_light_dat.period(), 100);
  ASSERT_EQ(
      traffic_light_dat.data_source(), os::v2x::device::DataSource::SIGNAL);
  ASSERT_EQ(
      traffic_light_dat.work_status(),
      os::v2x::device::DeviceWorkState::DEV_NORMAL);
  ASSERT_EQ(
      traffic_light_dat.control_mode(),
      os::v2x::device::ControlMode::LOCAL_FIX_CYCLE);
  ASSERT_EQ(traffic_light_dat.light_info_list_size(), 28);
#endif
}

}  // namespace device
}  // namespace v2x
}  // namespace os
