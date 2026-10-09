/*********************************************************************************
 * @file		ipcamera_component.h
 * @brief		IP Camera组件头文件
 * @details		IP Camera组件的头文件定义，包含组件类声明和接口定义
 * @author		ChangXuhui
 * @date		2025/7/4
 * @copyright	Copyright (c) 2025 The Airos Authors.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2025/7/4  ChangXuhui      1.0          ————               IP Camera组件头文件
 *
 * @endverbatim
 ********************************************************************************/

#pragma once

#include <thread>

#include "base/device_connect/ipcamera/device_base.h"
#include "middleware/runtime/src/air_middleware_component.h"
#include "middleware/device_service/framework/proto/ipcamera_config.pb.h"

namespace os {
namespace v2x {
namespace device {

class AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)
    : public airos::middleware::ComponentAdapter<os::v2x::device::ipcamera::IpCameraReceiveData> {
 public:
  AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)()           = default;
  ~AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)() override = default;

  bool Init() override;
  bool Proc(const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& recv_data)
      override;

 private:
  void MultiStreamCallBack(const std::string& stream_id, const IpCameraDataType& data);

 private:
  std::unique_ptr<IpCameraDevice> device_;
  std::unique_ptr<std::thread> task_;
  os::v2x::device::ipcamera::Config conf_;
};

REGISTER_AIROS_COMPONENT_CLASS(IpCameraComponent, os::v2x::device::ipcamera::IpCameraReceiveData);

}  // namespace device
}  // namespace v2x
}  // namespace os 