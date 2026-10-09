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

#include "rsu_component.h"

#include <experimental/filesystem>

#include "base/common/log.h"
#include "base/common/time_util.h"
#include "base/device_connect/rsu/device_factory.h"
#include "base/plugin/modules_loader/dynamic_loader.h"
#include "base/work_param/configer_om_work_param.h"
#include "base/common/auth/Authenticator.h"

namespace os {
namespace v2x {
namespace device {

bool AIROS_COMPONENT_CLASS_NAME(RSUComponent)::Init() {

#if ENABLE_ENCRYPTION
  auto omWorkParamConfiger = airos::base::workparam::WorkParam::getWorkParamFromFile();
  std::string license = omWorkParamConfiger.mecDeviceWorkParam.license;
  RSU_SERVICE_LOG_INFO << "License is: " << license << std::endl;
  int result = FusionService::Authenticator::GetInstance().Authorize(license);
  if (result != 0)
  {
    RSU_SERVICE_LOG_ERROR << "License generated fail, error code:  " << result;
    exit(1);
  }
#endif

  WriteBackFromCommParam();

  airos::base::DynamicLoader::GetInstance().LoadDevicePackage(
      "device/lib/rsu/");
  if (!LoadConfig(&conf_)) {
    RSU_SERVICE_LOG_ERROR << "load component proto config error";
    return false;
  }

  device_ = RSUDeviceFactory::Instance().GetUnique(
      conf_.device(),
      std::bind(
          &AIROS_COMPONENT_CLASS_NAME(RSUComponent)::CallBack,
          this,
          std::placeholders::_1));

  if (device_ == nullptr || !device_->Init(conf_.config_file())) {
    RSU_SERVICE_LOG_ERROR << "device_ init error";
    return false;
  }

  task_.reset(new std::thread([&]() {
    device_->Start();
  }));

  return true;
}

bool AIROS_COMPONENT_CLASS_NAME(RSUComponent)::Proc(
    const std::shared_ptr<const os::v2x::device::RSUData>& recv_data) {

    if (os::v2x::device::RSU_RSM == recv_data->type()) {
    recv_total_obj_num += recv_data->obj_num();
    // RSU_SERVICE_LOG_INFO << "sumarry: Recv obj num: " << recv_data->obj_num() << ", recv total obj num: " << recv_total_obj_num;

    if (recv_data->has_end_flag() && recv_data->end_flag()) {
      RSU_SERVICE_LOG_INFO << "sumarry: Expected recv total obj num: "
            << recv_data->total_obj_num()
            << ", Actual recv obj num: " << recv_total_obj_num
            << ", Obj loss rate: " << std::fixed
            << (recv_data->total_obj_num() - recv_total_obj_num) *
                   1.0 / recv_data->total_obj_num();

      double time_now = airos::base::TimeUtil::GetCurrentTime();
      RSU_SERVICE_LOG_INFO << "summary: Send timestamp: " << std::fixed
            << recv_data->message_timestamp()
            << " s, recv timestamp: " << time_now << " s, delay: "
            << time_now - recv_data->message_timestamp() << " s";
    }
  }

  device_->WriteToDevice(recv_data);
  return true;
}

void AIROS_COMPONENT_CLASS_NAME(RSUComponent)::CallBack(
    const RSUPBDataType& data) {
  Send("/airos/device/rsu_out", data);
}

void AIROS_COMPONENT_CLASS_NAME(RSUComponent)::WriteBackFromCommParam() {
  // 获取工参表
  auto omWorkParamConfiger =
      airos::base::workparam::WorkParam::getWorkParamFromFile();

  std::string filePath = "/home/airos/param/device/rsu/standard_rsu.yaml";

  // 读取YAML文件
  YAML::Node config = YAML::LoadFile(filePath);
  // 修改字段
  config["ip"] = omWorkParamConfiger.mecDeviceWorkParam.rsuEuhtIp;
  config["port"] = omWorkParamConfiger.mecDeviceWorkParam.rsuEuhtPort;
  config["local_ip"] = omWorkParamConfiger.mecDeviceWorkParam.mecIp;

  // 将修改后的内容保存回文件
  std::ofstream fout(filePath);
  fout << config;
  RSU_SERVICE_LOG_INFO << "Rsu/Euht ip: " << config["ip"] << ",port: " << config["port"]
        << ", local ip: " << config["local_ip"];
  RSU_SERVICE_LOG_INFO << "Configuration updated and saved to " << filePath;
}

}  // namespace device
}  // namespace v2x
}  // namespace os
