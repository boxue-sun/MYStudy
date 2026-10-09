/*********************************************************************************
 * @file		ipcamera_component.cc
 * @brief		IP Camera组件实现
 * @details		IP Camera组件的具体实现，提供设备管理和数据流处理功能
 * @author		ChangXuhui
 * @date		2025/7/4
 * @copyright	Copyright (c) 2025 The Airos Authors.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2025/7/4  ChangXuhui      1.0          ————               IP Camera组件实现
 *
 * @endverbatim
 ********************************************************************************/

#include "ipcamera_component.h"

#include "base/common/log.h"
#include "base/common/time_util.h"
#include "base/device_connect/ipcamera/device_factory.h"
#include "base/plugin/modules_loader/dynamic_loader.h"

namespace os {
namespace v2x {
namespace device {

bool AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)::Init() {
  // 加载设备库
  airos::base::DynamicLoader::GetInstance().LoadDevicePackage(
      "device/lib/ipcamera/");
      
  if (!LoadConfig(&conf_)) {
    IPCAMERA_SERVICE_LOG_ERROR << "load component proto config error";
    return false;
  }

  IPCAMERA_SERVICE_LOG_INFO << "device:" << conf_.device();
  
  // 创建设备实例，使用多路流回调
  device_ = IpCameraDeviceFactory::Instance().GetUnique(
      conf_.device(),
      std::bind(
          &AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)::MultiStreamCallBack,
          this,
          std::placeholders::_1,
          std::placeholders::_2));

  if (device_ == nullptr || !device_->Init(conf_.config_file())) {
    IPCAMERA_SERVICE_LOG_ERROR << "device_ init error";
    return false;
  }

  IPCAMERA_SERVICE_LOG_INFO << "device init success!";

  // 启动设备线程
  task_.reset(new std::thread([&]() {
    device_->Start();
  }));

  return true;
}

bool AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)::Proc(
    const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& recv_data) {
  // 将接收到的数据写入设备（用于控制命令等）
  if (device_ && recv_data) {
    device_->WriteToDevice(recv_data);
  }
  return true;
}

void AIROS_COMPONENT_CLASS_NAME(IpCameraComponent)::MultiStreamCallBack(
    const std::string& stream_id, const IpCameraDataType& data) {
  // 发送压缩图像数据到对应的话题
  Send(stream_id, data);
}

}  // namespace device
}  // namespace v2x
}  // namespace os 