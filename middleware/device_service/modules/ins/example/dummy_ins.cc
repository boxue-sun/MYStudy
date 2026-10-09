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

#include "dummy_ins.h"

#include <chrono>
#include <fstream>
#include <thread>

#include "base/device_connect/ins/device_factory.h"

namespace os {
namespace v2x {
namespace device {

bool DummyIns::Init(const std::string& config_file) {
  std::ofstream fs;
  fs.open(config_file, std::ios::in);
  if (!fs.is_open()) {
    return false;
  }
  /*
    解析config_file参数，初始化相机各项参数
    code here...

  */
  return true;
}

void DummyIns::Start() {
  int i = 0;
  while (i < 5) {
    // 设备数据产生与格式化制备
    auto data1 = std::make_shared<AoGPSFrame>();
    auto data2 = std::make_shared<AoIMUFrame>();

    data1->set_altitude(8888);
    data1->set_climb(5);
    data1->set_direction(120);
    data1->set_hacc(1);
    data1->set_hdop(1);
    data1->set_latitude(40);
    data1->set_longitude(120);
    data1->mutable_linearvelocity()->set_x(12);
    data1->mutable_linearvelocity()->set_y(34);
    data1->mutable_linearvelocity()->set_z(56);
    data1->set_timestamp_us(i * 1e6);
    data1->mutable_utcdata()->set_year(2000);
    data1->mutable_utcdata()->set_month(1);
    data1->mutable_utcdata()->set_day(1);
    data1->mutable_utctime()->set_hour(12);
    data1->mutable_utctime()->set_minute(33);
    data1->mutable_utctime()->set_second_s(44);
    data1->set_vacc(1);
    data1->set_vdop(1);
    data1->set_sequence_num(i);
    data1->set_type(AoGPSFrame::GOOD);

    data2->mutable_acceleration()->set_x(5.5);
    data2->mutable_acceleration()->set_y(6.6);
    data2->mutable_acceleration()->set_z(1.0);
    data2->set_heading(44);
    data2->set_headingtype(HeadingType::MAGNETICNORTH);
    data2->set_magnetometer(50);
    data2->set_orientation(60);
    data2->mutable_orientationquaternion()->set_qx(11);
    data2->mutable_orientationquaternion()->set_qy(22);
    data2->mutable_orientationquaternion()->set_qz(33);
    data2->mutable_orientationquaternion()->set_qw(44);
    data2->set_timestamp_us((i + 1) * 1e6);
    data2->mutable_angularvelocity()->set_x(1.1);
    data2->mutable_angularvelocity()->set_y(2.2);
    data2->mutable_angularvelocity()->set_z(3.3);
    data2->set_sequence_num(i + 1);
    ++i;

    // 将结构化数据输出给回调函数
    sender_1_(data1);
    sender_2_(data2);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

// 将DummyIns设备以”dummy_ins“名称注册给ins设备工厂
V2XOS_INS_REG_FACTORY(DummyIns, "dummy_ins");

}  // namespace device
}  // namespace v2x
}  // namespace os
