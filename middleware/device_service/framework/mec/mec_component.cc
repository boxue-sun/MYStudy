/*
 * @Author: zhangenwei
 * @Date: 2024-01-20 11:05:23
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-01-20 14:09:13
 * @Description:
 * @FilePath: /airos/base/device_connect/mec/cictci/mec_component.cc
 */

#include "mec_component.h"

#include "base/common/log.h"
#include "base/device_connect/mec/device_factory.h"
#include "base/work_param/configer_om_work_param.h"
#include "base/common/auth/Authenticator.h"

namespace os {
namespace v2x {
namespace device {

bool AIROS_COMPONENT_CLASS_NAME(MECComponent)::Init()
{

#if ENABLE_ENCRYPTION
  auto omWorkParamConfiger = airos::base::workparam::WorkParam::getWorkParamFromFile();
  std::string license = omWorkParamConfiger.mecDeviceWorkParam.license;
  MEC_SERVICE_LOG_INFO << "License is: " << license << std::endl;
  int result = FusionService::Authenticator::GetInstance().Authorize(license);
  if (result != 0)
  {
    MEC_SERVICE_LOG_ERROR << "License generated fail, error code:  " << result;
    exit(1);
  }
#endif

  if (!LoadConfig(&conf_))
  {
    MEC_SERVICE_LOG_ERROR << "load component proto config error";
    return false;
  }
  MEC_SERVICE_LOG_INFO << "device:" << conf_.device();
  device_ = MecDeviceFactory::Instance().GetUnique(conf_.device(),
      std::bind(&AIROS_COMPONENT_CLASS_NAME(MECComponent)::CallBack, this, std::placeholders::_1));

  if (device_ == nullptr || !device_->Init(conf_.config_file()))
  {
    MEC_SERVICE_LOG_ERROR << "device_ init error";
    return false;
  }
  else
  {
    MEC_SERVICE_LOG_INFO << "device init success!";
  }

  task_.reset(new std::thread([&]() { device_->Start(); }));

  return true;
}

bool AIROS_COMPONENT_CLASS_NAME(MECComponent)::Proc(
    const std::shared_ptr<const airos::usecase::EventOutputResult>& recv_data) {
  device_->WriteToDevice(recv_data);
  return true;
}

void AIROS_COMPONENT_CLASS_NAME(MECComponent)::CallBack(const MecDataType& data)
{
  Send("/v2x/usecase", data);
  MEC_SERVICE_LOG_WARN << "[mec-out] data:" << data->DebugString();
  // 存在感知目标, 通过/airos/perception/obstacles发布
  if (!data->perception_obstacle().empty()) {
    std::shared_ptr<airos::perception::PerceptionObstacles>
        perception_obstacles =
            std::make_shared<airos::perception::PerceptionObstacles>();
    // 复制 perception_obstacle 字段
    for (const auto& obstacle : data->perception_obstacle()) {
      auto* new_obstacle = perception_obstacles->add_perception_obstacle();
      new_obstacle->CopyFrom(obstacle);
    }

    // 复制 header 字段
    if (data->has_header()) {
      perception_obstacles->mutable_header()->CopyFrom(data->header());
    }
    MEC_SERVICE_LOG_WARN << "[mec-out] Send topic: /airos/perception/obstacles ";
      Send("/airos/perception/obstacles", perception_obstacles);
  }
    if (data->has_mec_sensor_data_monitor())
    {
        const auto& mec_sensor_data_monitor = data->mec_sensor_data_monitor();

        auto* monitor_rsap_response =  output_monitor_->mutable_rsap_response();

        monitor_rsap_response->set_tag(airos::monitor::MonitorMsgTag::MONITOR_TAG_SENSOR_DATA_IN);
        monitor_rsap_response->set_timestamp(mec_sensor_data_monitor.timestamp());
        monitor_rsap_response->set_sensor_data_in_port(mec_sensor_data_monitor.sensor_data_in_port());
        monitor_rsap_response->set_sensor_data_in_flag(mec_sensor_data_monitor.sensor_data_in_flag());
        monitor_rsap_response->set_sensor_data_in_obj_package(mec_sensor_data_monitor.sensor_data_in_obj_package());
        monitor_rsap_response->set_sensor_data_in_obj_num(mec_sensor_data_monitor.sensor_data_in_obj_num());

        MEC_SERVICE_LOG_WARN << "[monitor-out] Send topic: /v2x/monitor,[data]" << monitor_rsap_response->ShortDebugString();
        Send("/v2x/monitor", output_monitor_);
    }
    if (data->has_mec_sensor_data_monitor())
    {
      const auto& mec_sensor_data_monitor = data->mec_sensor_data_monitor();
      if (mec_sensor_data_monitor.sensor_data_in_flag())
      {
        auto* md_sensor_objs =  mec_monitor_->mutable_md_sensor_objs();
        md_sensor_objs->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_SENSOR_OBJ);
        md_sensor_objs->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
        MEC_SERVICE_LOG_WARN << "[mec-monitor-out] Send topic: /v2x/mec/om/check/rsap/data_,[data]" << mec_monitor_->ShortDebugString();
        Send("/v2x/mec/om/check/rsap/data_", mec_monitor_);
      }
    }
}

}  // namespace device
}  // namespace v2x
}  // namespace os
