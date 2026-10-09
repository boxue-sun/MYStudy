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

#include "traffic_light_service_component.h"

#include <jsoncpp/json/json.h>

#include "base/common/log.h"
#include "base/work_param/configer_om_work_param.h"
#include "base/common/auth/Authenticator.h"
#include "traffic_light_gflags.h"

namespace os {
namespace v2x {
namespace service {

bool AIROS_COMPONENT_CLASS_NAME(TrafficLightServiceComponent)::IsLightUnChanged(
    int light_id, ::airos::trafficlight::LightState status,
    bool has_countdown) {
  // 255s
  const static int MAX_LIGHT_DURATION = 2550;

  if (has_countdown || light_duration_record_[light_id].status != status) {
    light_duration_record_[light_id].countdown = 0;
  } else {
    light_duration_record_[light_id].countdown++;
  }
  light_duration_record_[light_id].status = status;
  if (light_duration_record_[light_id].countdown >= MAX_LIGHT_DURATION) {
    light_duration_record_[light_id].countdown = MAX_LIGHT_DURATION;
    return true;
  }
  return false;
}

bool AIROS_COMPONENT_CLASS_NAME(TrafficLightServiceComponent)::LoadPhaseTable(
    std::string phase_to_direction_file) {
  Json::Reader reader;
  Json::Value root;

  std::ifstream ifs(phase_to_direction_file);
  if (!ifs.is_open()) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "load old_phase to direction file failed!"
              << phase_to_direction_file;
    return false;
  }

  if (reader.parse(ifs, root, false)) {
    int phase_size     = root["phase"].size();
    int direction_size = root["direction"].size();
    if (root["phase"].isNull() || root["direction"].isNull() ||
        phase_size != direction_size) {
      TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "phase_to_direction file invalid: phase and direction not "
                   "match";
      ifs.close();
      return false;
    }
    for (int i = 0; i < phase_size; ++i) {
      map_direction_to_phase_[root["direction"][i].asInt()] =
          uint8_t(root["phase"][i].asInt());
    }

    if (root.isMember("round")) {
      int round_size = root["round"].size();
      if (phase_size != round_size) {
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR
            << "phase_to_direction file invalid: round and phase not match";
        ifs.close();
        return false;
      }
      for (int i = 0; i < phase_size; ++i) {
        map_direction_to_round_[root["direction"][i].asInt()] =
            root["round"][i].asInt();
      }
    }
  } else {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "json parse failed!";
    ifs.close();
    return false;
  }

  ifs.close();
  return true;
}

void AIROS_COMPONENT_CLASS_NAME(TrafficLightServiceComponent)::TransformPhase(
    const std::shared_ptr<os::v2x::device::TrafficLightBaseData>&
        traffic_light_data_pb) {
  if (nullptr == traffic_light_data_pb) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "Origin traffic light data invalid!";
    return;
  }

  auto origin_light_infos = traffic_light_data_pb->light_info_list();
  traffic_light_data_pb->clear_light_info_list();

  // Set direction id
  for (auto& item : map_direction_to_phase_) {
    auto phase_id = item.first;
    auto light_id = item.second;
    for (int i = 0; i < origin_light_infos.size(); ++i) {
      if (origin_light_infos[i].light_id() == light_id) {
        // set old_phase id
        auto new_light_info = traffic_light_data_pb->add_light_info_list();
        *new_light_info     = origin_light_infos[i];
        new_light_info->set_light_id(phase_id);

        // Set round id
        if ((map_direction_to_round_.size() > 0) &&
            (map_direction_to_round_.count(phase_id) > 0)) {
          os::v2x::device::LightType temp_light_type =
              os::v2x::device::LightType::MOTOR_VEHICLE;
          if (map_direction_to_round_[phase_id] == 0) {
            switch (phase_id % 5) {
              case 1:
                temp_light_type =
                    os::v2x::device::LightType::STRAIGHT_DIRECTION;
                break;
              case 2:
                temp_light_type = os::v2x::device::LightType::LEFT_DIRECTION;
                break;
              case 3:
                temp_light_type = os::v2x::device::LightType::RIGHT_DIRECTION;
                break;
              case 4:
                temp_light_type = os::v2x::device::LightType::TURN_DIRECTION;
                break;
              case 5:
                temp_light_type = os::v2x::device::LightType::MOTOR_VEHICLE;
                break;
            }
          } else {
            temp_light_type = os::v2x::device::LightType::MOTOR_VEHICLE;
          }
          new_light_info->set_light_type(temp_light_type);
        }
      }
    }
  }
}

bool AIROS_COMPONENT_CLASS_NAME(TrafficLightServiceComponent)::
    TrafficLightDataPb2ServicePb(
        const std::shared_ptr<os::v2x::device::TrafficLightBaseData>&
            traffic_light_data_pb,
        std::shared_ptr<::airos::trafficlight::TrafficLightServiceData>&
            traffic_light_service_pb) {
  TRAFFIC_LIFGT_SERVICE_LOG_INFO << "transform TrafficLightBaseData to TrafficLightServiceData";
  if (nullptr == traffic_light_data_pb || nullptr == traffic_light_service_pb) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "Input param is nullptr!";
    return false;
  }

  traffic_light_service_pb->set_city_code(city_code_);
  traffic_light_service_pb->set_region_id(region_id_);
  traffic_light_service_pb->set_cross_id(cross_id_);  // rsu_intersection_id

  // Set traffic_light member
  auto traffic_light = traffic_light_service_pb->mutable_traffic_light();
  traffic_light->set_time_stamp(traffic_light_data_pb->time_stamp());
  if (traffic_light_data_pb->has_period()) {
    traffic_light->set_period(traffic_light_data_pb->period());
  }
  traffic_light->set_data_source(
      (::airos::trafficlight::DataSource)traffic_light_data_pb->data_source());

  // set confidence
  if (traffic_light_data_pb->data_source() ==
      os::v2x::device::DataSource::SIGNAL) {
    traffic_light->set_confidence(100);
  } else {
    traffic_light->set_confidence(80);
  }

  // set device info member
  if (traffic_light_data_pb->has_vendor()) {
    traffic_light->mutable_device_info()->set_vendor(
        traffic_light_data_pb->vendor());
  }
  if (traffic_light_data_pb->has_model()) {
    traffic_light->mutable_device_info()->set_model(
        traffic_light_data_pb->model());
  }
  if (traffic_light_data_pb->has_software_version()) {
    traffic_light->mutable_device_info()->set_software_version(
        traffic_light_data_pb->software_version());
  }
  traffic_light->mutable_device_info()->set_work_status(
      (::airos::trafficlight::DeviceWorkState)
          traffic_light_data_pb->work_status());
  traffic_light->mutable_device_info()->set_control_mode(
      (::airos::trafficlight::ControlMode)
          traffic_light_data_pb->control_mode());

  // set origin data list member
  *traffic_light->mutable_original_data_list() =
      traffic_light_data_pb->original_data_list();

  // Set phase member
  for (auto& old_light_info : traffic_light_data_pb->light_info_list()) {
    auto new_phase = traffic_light->add_phase();
    new_phase->set_light_id(old_light_info.light_id());
    new_phase->set_light_type(
        (::airos::trafficlight::LightType)old_light_info.light_type());
    new_phase->set_light_status(
        (::airos::trafficlight::LightState)old_light_info.light_status());

    if (old_light_info.has_count_down()) {
      new_phase->set_count_down(old_light_info.count_down());
    }

    if (IsLightUnChanged(
            old_light_info.light_id(),
            (::airos::trafficlight::LightState)old_light_info.light_status(),
            old_light_info.has_count_down())) {
      new_phase->set_light_unchanged(true);
    } else {
      new_phase->set_light_unchanged(false);
    }

    // Set step_info_list member
    for (auto& old_step_info : old_light_info.step_info_list()) {
      auto new_step_info = new_phase->add_step_info_list();
      new_step_info->set_light_status(
          (::airos::trafficlight::LightState)old_step_info.light_status());
      new_step_info->set_duration(old_step_info.duration());
    }

    // Set right_way_time member
    new_phase->set_right_way_time(old_light_info.right_way_time());

    // Set entry_direction 
    new_phase->set_entry_direction(old_light_info.entry_direction());
  }

  return true;
}

bool AIROS_COMPONENT_CLASS_NAME(TrafficLightServiceComponent)::Init() {
  TRAFFIC_LIFGT_SERVICE_LOG_INFO
      << "AIROS_COMPONENT_CLASS_NAME(TrafficLightServiceComponent) Init...";

  auto omWorkParamConfiger = airos::base::workparam::WorkParam::getWorkParamFromFile();
#if ENABLE_ENCRYPTION
  std::string license = omWorkParamConfiger.mecDeviceWorkParam.license;
  TRAFFIC_LIFGT_SERVICE_LOG_INFO << "License is: " << license << std::endl;
  // std::string license = "cictci_test|1767196799000|1|0|7oa0T3ZxHgDyPVAG|1009b861083a438fe89ce8ab954ee49b|E64FD69136845D155AF4434037A4DFC3FD92C9E043079996E98DAE76C791320AEF135ED802C19A64FF4F1136B8B516BEC9EA041618A99DCF11CA56D5D282205FB607292E8C6DB4B15A592D9F016CC13A11EFE21E15D42FEF6E4953A2D79DB2F1";
  int result = FusionService::Authenticator::GetInstance().Authorize(license);
  if (result != 0)
  {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "License generated fail, error code:  " << result;
    exit(1);
  }
#endif

  city_code_ = FLAGS_city_code;

  // region_id_ = FLAGS_intersection_region_spat;
  // cross_id_  = FLAGS_rsu_intersection_id;

  // 使用工参表中提供的数据
  // auto omWorkParamConfiger =
  //     airos::base::workparam::WorkParam::getWorkParamFromFile();
  region_id_ = omWorkParamConfiger.mecDeviceWorkParam.regionID;
  cross_id_ = omWorkParamConfiger.mecDeviceWorkParam.crossID;
  TRAFFIC_LIFGT_SERVICE_LOG_INFO << "Get is from WorkParam, region_id: " << region_id_
           << ", cross_id: " << cross_id_;

  // load phase transform json
  LoadPhaseTable(FLAGS_phase_to_direction_path);
  return true;
}

bool AIROS_COMPONENT_CLASS_NAME(TrafficLightServiceComponent)::Proc(
    const std::shared_ptr<const os::v2x::device::TrafficLightBaseData>&
        traffic_light_data_pb) {
  TRAFFIC_LIFGT_SERVICE_LOG_INFO << "Received traffic light from device";
  if (!traffic_light_data_pb) {
    TRAFFIC_LIFGT_SERVICE_LOG_WARN << "Received traffic light data failed!";
    return false;
  }

  std::shared_ptr<os::v2x::device::TrafficLightBaseData>
      traffic_light_device_pb =
          std::make_shared<os::v2x::device::TrafficLightBaseData>(
              *traffic_light_data_pb);

  // transform to baidu phase
  // TransformPhase(traffic_light_device_pb);

  // transform TrafficLightBaseData to TrafficLightServiceData
  std::shared_ptr<::airos::trafficlight::TrafficLightServiceData>
      traffic_light_service_pb =
          std::make_shared<::airos::trafficlight::TrafficLightServiceData>();
  if (!TrafficLightDataPb2ServicePb(
          traffic_light_device_pb, traffic_light_service_pb)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "transform Traffic Light device data to service data failed!";
    return false;
  }

  // Send data
  Send("/airos/service/traffic_light/data", traffic_light_service_pb);
  return true;
}

}  // namespace service
}  // namespace v2x
}  // namespace os
