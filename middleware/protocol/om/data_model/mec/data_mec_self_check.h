/*********************************************************************************
* @file		maintenance_management_ota_data.h
 * @brief		maintenance_management_ota_data belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-21
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-4-21 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_MEC_SELF_CHECK_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_MEC_SELF_CHECK_H
#include "middleware/protocol/om_common/data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_MEC
using namespace os::v2x::protocol::om::common;

struct TopicCCIndexTm
{
    std::string Topic_Trajectories  = "trajectories/"; // mec向平台发送 实时轨迹数据
    std::string Topic_VehiclePass   = "vehiclePass/";   // mec向平台发送 实时过车数据
    std::string Topic_QueueUp       = "queueUp/";       // mec向平台发送 实时排队数据
    std::string Topic_AreaState     = "areaState/";// mec向平台发送 实时区域状态数据
    std::string Topic_Overflow      = "overflow/";// mec向平台发送 实时溢出数据
    std::string Topic_Outlane       = "outlane/";// mec向平台发送 实时出口通道数据
    std::string Topic_Statistics    = "statistics/";// mec向平台发送 定时统计数据
    std::string Topic_Evaluations   = "evaluations/";// mec向平台发送 定时评价数据
    std::string Topic_Nonmotor      = "nonmotor/";// mec向平台发送 定时评价数据
    std::string Topic_DeviceStatus = "deviceStatus/";// mec向平台发送 设备状态数据
    std::string Topic_Pulse         = "pulse/";   // mec向平台发送, 实时脉冲数据

    /**
 * @brief 将结构体内容转换为字符串用于打印日志
 * @return 格式化后的字符串
 */
    std::string to_string() const
    {
        std::stringstream ss;
        ss << "TopicCCIndexTm Configuration:\n"
           << "  [Trajectories] : " << Topic_Trajectories << "\n"
           << "  [VehiclePass]  : " << Topic_VehiclePass << "\n"
           << "  [QueueUp]      : " << Topic_QueueUp << "\n"
           << "  [AreaState]    : " << Topic_AreaState << "\n"
           << "  [Overflow]     : " << Topic_Overflow << "\n"
           << "  [Outlane]      : " << Topic_Outlane << "\n"
           << "  [Statistics]   : " << Topic_Statistics << "\n"
           << "  [Evaluations]  : " << Topic_Evaluations << "\n"
           << "  [Nonmotor]     : " << Topic_Nonmotor << "\n"
           << "  [DeviceStatus] : " << Topic_DeviceStatus << "\n"
           << "  [Pulse]        : " << Topic_Pulse ;
        return ss.str();
    }
};


struct TopicCCIndexSt
{
    std::string Topic_Static_Query_Ack = "query/ack/";
    std::string Topic_Static_Update = "update";
    /**
* @brief 将结构体内容转换为字符串用于打印日志
* @return 格式化后的字符串
*/
    std::string to_string() const
    {
        std::stringstream ss;
        ss << "TopicCCIndexTm Configuration:\n"
           << "  [Static_Update_Ack] : " << Topic_Static_Query_Ack << "\n"
           << "  [Static_Update]  : " << Topic_Static_Update ;
        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif