/*
 * @Author: zhangenwei
 * @Date: 2024-01-21 10:35:41
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-01-21 10:35:41
 * @Description:
 * @FilePath: /airos/base/device_connect/mec/device_factory.h
 */
#pragma once

#include <functional>
#include <map>
#include "glog/logging.h"
#include "device_base.h"
namespace os {
namespace v2x {
namespace device {

/**
 * @brief  Mec设备注册工厂
 */
class MecDeviceFactory {
 public:
  using CONSTRUCT = std::function<MecDevice *(const MecCallBack &cb)>;

  template <typename Inherit>
  class Register_t {
   public:
    Register_t(const std::string &key) {
      MecDeviceFactory::Instance().map_.emplace(
          key, [](const MecCallBack &cb) { return new Inherit(cb); });
    }
  };

  /**
   * @brief      用于获取指定Mec设备实例的unique指针
   * @param[in]  key 指定Mec设备名
   * @retval     指定Mec设备实例的unique指针
   */
  std::unique_ptr<MecDevice> GetUnique(const std::string &key,
                                       const MecCallBack &cb);

  /**
   * @brief      用于获取指定Mec设备实例的shared指针
   * @param[in]  key 指定Mec设备名
   * @retval     指定Mec设备实例的shared指针
   */
  std::shared_ptr<MecDevice> GetShared(const std::string &key,
                                       const MecCallBack &cb);

  /**
   * @brief      用于获取Mec设备工厂实例（单例）
   * @retval     Mec设备工厂实例
   */
  static MecDeviceFactory &Instance();

 private:
  MecDevice *Produce(const std::string &key, const MecCallBack &cb);

  MecDeviceFactory(){};

  MecDeviceFactory(const MecDeviceFactory &) = delete;

  MecDeviceFactory(MecDeviceFactory &&) = delete;

 private:
  std::map<std::string, CONSTRUCT> map_;
};

#define V2XOS_MEC_REG(T) v2xos_reg_func_str_##T##_
/**
 * @brief      用于注册指定Mec设备
 * @param[in]  T 具体Mec设备子类型
 * @param[in]  key 注册的Mec设备名字
 */
#define V2XOS_MEC_REG_FACTORY(T, key)                       \
  static MecDeviceFactory::Register_t<T> V2XOS_MEC_REG(T)(  \
      key);

}  // namespace device
}  // namespace v2x
}  // namespace os