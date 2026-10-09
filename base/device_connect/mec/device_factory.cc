/*
 * @Author: zhangenwei
 * @Date: 2024-01-21 10:39:40
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-01-22 11:20:40
 * @Description:
 * @FilePath: /airos/base/device_connect/mec/device_factory.cc
 */
#include "base/device_connect/mec/device_factory.h"

namespace os {
namespace v2x {
namespace device {

MecDeviceFactory &MecDeviceFactory::Instance()
{
    static MecDeviceFactory instance_;
    return instance_;
}

std::unique_ptr <MecDevice> MecDeviceFactory::GetUnique(const std::string &key, const MecCallBack &cb)
{
    return std::unique_ptr<MecDevice>(Produce(key, cb));
}

std::shared_ptr <MecDevice> MecDeviceFactory::GetShared(const std::string &key, const MecCallBack &cb)
{
    return std::shared_ptr<MecDevice>(Produce(key, cb));
}

MecDevice *MecDeviceFactory::Produce(const std::string &key, const MecCallBack &cb)
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