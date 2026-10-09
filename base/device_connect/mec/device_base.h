/*
 * @Author: zhangenwei
 * @Date: 2024-01-21 10:46:21
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-01-21 10:46:21
 * @Description:
 * @FilePath: /airos/base/device_connect/mec/device_base.h
 */
#pragma once
#include <functional>
#include <memory>

#include "air_service/framework/proto/airos_usecase.pb.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @struct  MecDeviceState
 * @brief   Mec设备运行状态
 * @details
 */
enum class MecDeviceState
{
    UNKNOWN,   //未知
    RUNNING,  //运行
    STOP
};

using MecDataType = std::shared_ptr<const airos::usecase::EventOutputResult>;
using MecCallBack = std::function<void(const MecDataType &)>;

/**
 * @brief  Mec设备接口类
 */
class MecDevice
{
public:
    MecDevice() = default;

    /**
     * @brief      Mec设备构造函数
     * @param[in]  cb AirOS-edge框架注册的回调函数
     */
    explicit MecDevice(const MecCallBack &cb) : sender_(cb)
    {
    }

    virtual ~MecDevice() = default;

    /**
     * @brief      用于Mec设备初始化
     * @param[in]  config_file Mec设备初始化参数配置文件
     * @retval     初始化是否成功
     */
    virtual bool Init(const std::string &config_file) = 0;

    /**
     * @brief 用于启动Mec设备，产出AirOS-edge结构化的标准红绿灯采集输出数据
     */
    virtual void Start() = 0;

    /**
     * @brief      用于获取Mec设备状态
     * @retval     设备状态码红绿灯采集DeviceState
     */
    virtual MecDeviceState GetState() = 0;

    /**
     * @brief      用于将数据写入Mec设备状态
     * @param[in]  re_proto AirOS-edge结构化的标准红绿灯采集写入数据
     */
    virtual void WriteToDevice(
        const std::shared_ptr<const airos::usecase::EventOutputResult>&re_proto) = 0;

   protected:
    MecCallBack sender_;
};

}  // namespace device
}  // namespace v2x
}  // namespace os