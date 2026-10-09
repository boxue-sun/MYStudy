/*********************************************************************************
 * @file		device_factory.cc
 * @brief		IP Camera设备工厂类实现
 * @details		IP Camera设备工厂类的具体实现，提供单例模式和设备实例化功能
 * @author		ChangXuhui
 * @date		2025/7/4
 * @copyright	Copyright (c) 2025 The Airos Authors.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2025/7/4  ChangXuhui      1.0          ————               IP Camera设备工厂类实现
 *
 * @endverbatim
 ********************************************************************************/

#include "base/device_connect/ipcamera/device_factory.h"

namespace os {
namespace v2x {
namespace device {

IpCameraDeviceFactory &IpCameraDeviceFactory::Instance()
{
    static IpCameraDeviceFactory instance_;
    return instance_;
}

std::unique_ptr<IpCameraDevice> IpCameraDeviceFactory::GetUnique(const std::string &key, const IpCameraCallBack &cb)
{
    return std::unique_ptr<IpCameraDevice>(Produce(key, cb));
}

std::shared_ptr<IpCameraDevice> IpCameraDeviceFactory::GetShared(const std::string &key, const IpCameraCallBack &cb)
{
    return std::shared_ptr<IpCameraDevice>(Produce(key, cb));
}

IpCameraDevice *IpCameraDeviceFactory::Produce(const std::string &key, const IpCameraCallBack &cb)
{
    if (map_.find(key) == map_.end())
    {
        return nullptr;
    }
    return map_[key](cb);
}

}  // namespace device
}  // namespace v2x
}  // namespace os 
