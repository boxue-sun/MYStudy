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

#include "middleware/device_service/modules/lidar/example/dummy_lidar.h"

#include <chrono>
#include <fstream>
#include <sstream>

#include "base/device_connect/lidar/device_factory.h"

namespace os {
namespace v2x {
namespace device {

bool DummyLidar::Init(const LidarInitConfig& config) {
  /*
    解析config参数，初始化相机各项参数
    code here...

  */
  recv_thread_.reset(new std::thread(&DummyLidar::LidarGetData, this));
  return true;
}

void DummyLidar::LidarGetData() {
  int i = 0;
  while (i < 5) {
    // 设备数据产生与格式化制备
    auto data = std::make_shared<PointCloud>();
    /*
      填充格式化的输出数据
      code here...

    */
    data->mutable_header()->set_sequence_num(i);
    std::stringstream ss;
    ss << i;
    data->set_frame_id(ss.str());
    data->set_is_dense(i % 2 ? true : false);
    for (int j = 0; j < 3; ++j) {
      auto pt = data->add_point();
      pt->set_x(j + i * 1e1);
      pt->set_y(j + i * 1e1);
      pt->set_z(j + i * 1e1);
    }
    data->set_measurement_time(i + 1e6);
    data->set_width(7);
    data->set_height(2);
    ++i;

    // 将结构化数据输出给回调函数
    sender_(data);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

// 将DummyLidar设备以”dummy_lidar“名称注册给lidar设备工厂
V2XOS_LIDAR_REG_FACTORY(DummyLidar, "dummy_lidar");

}  // namespace device
}  // namespace v2x
}  // namespace os
