/*********************************************************************************
 * @file		device_base.h
 * @brief		IP Camera设备基类
 * @details		IP Camera设备接口定义，支持多路RTSP流接入
 * @author		ChangXuhui
 * @date		2025/7/4
 * @copyright	Copyright (c) 2025 The Airos Authors.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2025/7/4  ChangXuhui      1.0          ————               IP Camera设备基类开发
 *                                                             
 *
 * @endverbatim
 ********************************************************************************/
#pragma once
#include <functional>
#include <memory>

#include "base/device_connect/proto/ipcamera.pb.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @struct  IpCameraDeviceState
 * @brief   IP Camera设备运行状态
 * @details
 */
enum class IpCameraDeviceState
{
    UNKNOWN,   //未知
    RUNNING,  //运行
    STOP
};

using IpCameraDataType = std::shared_ptr<const os::v2x::device::ipcamera::CompressedImage>;
// 多路流回调函数类型
using IpCameraCallBack = std::function<void(const std::string& stream_id, const IpCameraDataType &)>;

/**
 * @brief  IP Camera设备接口类
 */
class IpCameraDevice
{
public:
    IpCameraDevice() = default;

    /**
     * @brief      IP Camera设备构造函数
     * @param[in]  cb AirOS-edge框架注册的多路流回调函数
     */
    explicit IpCameraDevice(const IpCameraCallBack &cb) : sender_(cb)
    {
    }

    virtual ~IpCameraDevice() = default;

    /**
     * @brief      用于IP Camera设备初始化
     * @param[in]  config_file IP Camera设备初始化参数配置文件
     * @retval     初始化是否成功
     */
    virtual bool Init(const std::string &config_file) = 0;

    /**
     * @brief 用于启动IP Camera设备，产出AirOS-edge结构化的标准图像采集输出数据
     */
    virtual void Start() = 0;

    /**
     * @brief      用于获取IP Camera设备状态
     * @retval     设备状态码图像采集DeviceState
     */
    virtual IpCameraDeviceState GetState() = 0;

    /**
     * @brief      用于将数据写入IP Camera设备状态
     * @param[in]  receive_data AirOS-edge结构化的IP Camera接收数据
     */
    virtual void WriteToDevice(
        const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& receive_data) = 0;

   protected:
    IpCameraCallBack sender_;
};

}  // namespace device
}  // namespace v2x
}  // namespace os
