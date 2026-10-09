/*********************************************************************************
 * @file		device_factory.h
 * @brief		IP Camera设备工厂类
 * @details		IP Camera设备注册工厂，支持动态设备注册和实例化
 * @author		ChangXuhui
 * @date		2025/7/4
 * @copyright	Copyright (c) 2025 The Airos Authors.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2025/7/4  ChangXuhui      1.0          ————               IP Camera设备工厂类开发
 *
 * @endverbatim
 ********************************************************************************/
#pragma once

#include <functional>
#include <map>
#include "device_base.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @brief  IP Camera设备注册工厂
 */
class IpCameraDeviceFactory {
 public:
  using CONSTRUCT = std::function<IpCameraDevice *(const IpCameraCallBack &cb)>;

  template <typename Inherit>
  class Register_t {
   public:
    Register_t(const std::string &key) {
      IpCameraDeviceFactory::Instance().map_.emplace(
          key, [](const IpCameraCallBack &cb) { return new Inherit(cb); });
    }
  };

  /**
   * @brief      用于获取指定IP Camera设备实例的unique指针
   * @param[in]  key 指定IP Camera设备名
   * @retval     指定IP Camera设备实例的unique指针
   */
  std::unique_ptr<IpCameraDevice> GetUnique(const std::string &key,
                                           const IpCameraCallBack &cb);

  /**
   * @brief      用于获取指定IP Camera设备实例的shared指针
   * @param[in]  key 指定IP Camera设备名
   * @retval     指定IP Camera设备实例的shared指针
   */
  std::shared_ptr<IpCameraDevice> GetShared(const std::string &key,
                                           const IpCameraCallBack &cb);


  /**
   * @brief      用于获取IP Camera设备工厂实例（单例）
   * @retval     IP Camera设备工厂实例
   */
  static IpCameraDeviceFactory &Instance();

 private:
  IpCameraDevice *Produce(const std::string &key, const IpCameraCallBack &cb);

  IpCameraDeviceFactory(){};

  IpCameraDeviceFactory(const IpCameraDeviceFactory &) = delete;

  IpCameraDeviceFactory(IpCameraDeviceFactory &&) = delete;

 private:
  std::map<std::string, CONSTRUCT> map_;
};

#define V2XOS_IPCAMERA_REG(T) v2xos_reg_func_str_##T##_
/**
 * @brief      用于注册指定IP Camera设备
 * @param[in]  T 具体IP Camera设备子类型
 * @param[in]  key 注册的IP Camera设备名字
 */
#define V2XOS_IPCAMERA_REG_FACTORY(T, key)                       \
  static IpCameraDeviceFactory::Register_t<T> V2XOS_IPCAMERA_REG(T)(  \
      key);

}  // namespace device
}  // namespace v2x
}  // namespace os 
