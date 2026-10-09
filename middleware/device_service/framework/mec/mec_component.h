/*
 * @Author: zhangenwei
 * @Date: 2024-01-20 9:15:07
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-01-20 10:25:18
 * @Description:
 * @FilePath: /middleware/device_service/mec/mec_component.h
 */

#pragma once

#include <thread>

#include "middleware/device_service/framework/proto/mec_config.pb.h"

#include "base/device_connect/mec/device_base.h"
#include "middleware/runtime/src/air_middleware_component.h"
#include "middleware/protocol/proto/monitor.pb.h"
#include "middleware/protocol/proto/monitor_mec.pb.h"
namespace os {
namespace v2x {
namespace device {
using namespace airos::monitor;
class AIROS_COMPONENT_CLASS_NAME(MECComponent)
    : public airos::middleware::ComponentAdapter<airos::usecase::EventOutputResult> {
 public:
  AIROS_COMPONENT_CLASS_NAME(MECComponent)() {
      output_monitor_ = std::make_shared<airos::monitor::MonitorResponse>();
      mec_monitor_ = std::make_shared<airos::monitor_mec::MonitorMec>();
  };
  ~AIROS_COMPONENT_CLASS_NAME(MECComponent)() override = default;

  bool Init() override;
  bool Proc(const std::shared_ptr<const airos::usecase::EventOutputResult>& recv_data) override;

 private:
  void CallBack(const MecDataType& data);

 private:
  std::unique_ptr<MecDevice> device_;
  std::unique_ptr<std::thread> task_;
  os::v2x::device::mec::Config conf_;

  std::shared_ptr <airos::monitor::MonitorResponse> output_monitor_ = nullptr;

  std::shared_ptr <airos::monitor_mec::MonitorMec> mec_monitor_ = nullptr;
};

REGISTER_AIROS_COMPONENT_CLASS(MECComponent, airos::usecase::EventOutputResult);

}  // namespace device
}  // namespace v2x
}  // namespace os
