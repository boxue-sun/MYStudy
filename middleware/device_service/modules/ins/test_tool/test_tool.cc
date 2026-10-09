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

#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "base/device_connect/ins/device_factory.h"
#include "yaml-cpp/yaml.h"

using os::v2x::device::GnssPBDataTypePtr;
using os::v2x::device::IMUPBDataTypePtr;
using os::v2x::device::InsDevice;
using os::v2x::device::InsDeviceFactory;

#define SHOW_DASH " ################## "
#define SHOW_TITLE(name) SHOW_DASH name SHOW_DASH

static void proc_gnss_data(const GnssPBDataTypePtr& data) {
  std::cout << SHOW_TITLE("GnssData") << std::endl;
  std::cout << "altitude: " << data->altitude() << std::endl;
  std::cout << "climb: " << data->climb() << std::endl;
  std::cout << "direction: " << data->direction() << std::endl;
  std::cout << "hacc: " << data->hacc() << std::endl;
  std::cout << "hdop: " << data->hdop() << std::endl;
  std::cout << "latitude: " << data->latitude() << std::endl;
  std::cout << "longitude: " << data->longitude() << std::endl;
  std::cout << "linearVelocity: "
            << "\n x: " << data->linearvelocity().x()
            << "\n y: " << data->linearvelocity().y()
            << "\n z: " << data->linearvelocity().z() << std::endl;
  std::cout << "timestamp_us: " << data->timestamp_us() << std::endl;
  std::cout << "utcData: "
            << "\n year: " << data->utcdata().year()
            << "\n month: " << data->utcdata().month()
            << "\n day: " << data->utcdata().day() << std::endl;
  std::cout << "utcTime: "
            << "\n hour: " << data->utctime().hour()
            << "\n minute: " << data->utctime().minute()
            << "\n second_s: " << data->utctime().second_s() << std::endl;
  std::cout << "vacc: " << data->vacc() << std::endl;
  std::cout << "vdop: " << data->vdop() << std::endl;
  std::cout << "sequence_num: " << data->sequence_num() << std::endl;
  std::cout << "type: " << static_cast<int>(data->type()) << std::endl;
}

static void proc_imu_data(const IMUPBDataTypePtr& data) {
  std::cout << SHOW_TITLE("ImuData") << std::endl;
  std::cout << "acceleration: "
            << "\n x: " << data->acceleration().x()
            << "\n y: " << data->acceleration().y()
            << "\n z: " << data->acceleration().z() << std::endl;
  std::cout << "heading: " << data->heading() << std::endl;
  std::cout << "headingtype: " << static_cast<int>(data->headingtype())
            << std::endl;
  std::cout << "magnetometer: " << data->magnetometer() << std::endl;
  std::cout << "orientation: " << data->orientation() << std::endl;
  std::cout << "orientationquaternion: "
            << "\n qx: " << data->orientationquaternion().qx()
            << "\n qy: " << data->orientationquaternion().qy()
            << "\n qz: " << data->orientationquaternion().qz()
            << "\n qw: " << data->orientationquaternion().qw() << std::endl;
  std::cout << "timestamp_us: " << data->timestamp_us() << std::endl;
  std::cout << "angularvelocity: "
            << "\n x: " << data->angularvelocity().x()
            << "\n y: " << data->angularvelocity().y()
            << "\n z: " << data->angularvelocity().z() << std::endl;
  std::cout << "sequence_num: " << data->sequence_num() << std::endl;
}

bool parse_config(
    const std::string& ex_conf, std::string& type_name, std::string& ins_name,
    std::string& ins_conf) {
  try {
    YAML::Node node = YAML::LoadFile(ex_conf);
    type_name       = node["device_type"].as<std::string>();
    ins_name        = node["ins_name"].as<std::string>();
    ins_conf        = node["ins_config"].as<std::string>();
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    return false;
  } catch (...) {
    std::cerr << "Parse yaml config failed: " << ex_conf << std::endl;
    return false;
  }

  return true;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Args Error!\nUsage: ins_test_tool conf_file1 conf_file2 ..."
              << std::endl;
    return 1;
  }

  std::map<std::string, std::shared_ptr<InsDevice>> device_name_map;
  std::string device_type, ins_name, ins_conf;
  for (int i = 1; i < argc; ++i) {
    if (!parse_config(argv[i], device_type, ins_name, ins_conf)) {
      std::cerr << "Parse conf_file failed! skip: " << argv[i] << std::endl;
      continue;
    }

    auto device = InsDeviceFactory::Instance().GetShared(
        device_type, proc_gnss_data, proc_imu_data);
    if (!device) {
      std::cerr << "Build device instance failed! skip: " << device_type
                << std::endl;
      continue;
    }
    if (!device->Init(ins_conf)) {
      std::cerr << "Init " << ins_name << " failed! skip: " << ins_name
                << std::endl;
      continue;
    }
    device_name_map[ins_name] = device;
  }

  std::map<std::string, std::unique_ptr<std::thread>> task_map;
  for (const auto& item : device_name_map) {
    ins_name     = item.first;
    auto& device = item.second;
    task_map.emplace(ins_name, new std::thread([device]() {
                       device->Start();
                     }));
  }

  for (auto& item : task_map) {
    auto& task = item.second;
    if (task->joinable()) {
      task->join();
    }
  }

  return 0;
}
