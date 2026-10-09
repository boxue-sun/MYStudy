/*********************************************************************************
 * @file		data_register.h
 * @brief		data_register belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-8-29
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 * 24-8-29 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef AIROS2_0_DATA_REGISTER_H
#define AIROS2_0_DATA_REGISTER_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "base/common/network/serializable_data.h"
NAMESPACE_START_OM_COMPONENT_DEVICE_STATUS
using namespace afl::base;
struct CameraRegister : public afl::base::SerializableData
{
    uint64_t timeStamp;             // 时间戳，UTC时间，单位为毫秒
    std::string seqNum;              // 会话唯一标识
    std::string deviceID;            // 设备唯一标识
    double longitude;                 // 02坐标系的经度
    double latitude;                  // 02坐标系的维度
    std::string manuFacturer;        // 厂商信息
    std::string model;               // 型号信息
    std::string swVersion;           // 软件版本信息
    int httpPort;                    // http端口
    int httpsPort;                   // https端口
    int rtspPort;                    // Rtsp服务端口
    int chaNum;                      // 通道总数
    int state;                       // 通道状态，0：在线，1：离线
    int streamNum;                   // 该通道码流个数
    private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(timeStamp,                       "A_timeStamp",                       j, false);
        JsonSerialize(seqNum,                          "B_seqNum",                          j, false);
        JsonSerialize(deviceID,                        "C_deviceID",                        j, false);
        JsonSerialize(longitude,                       "D_longitude",                       j, false);
        JsonSerialize(latitude,                        "E_latitude",                        j, false);
        JsonSerialize(manuFacturer,                   "F_manuFacturer",                   j, false);
        JsonSerialize(model,                           "G_model",                           j, false);
        JsonSerialize(swVersion,                       "H_swVersion",                       j, false);
        JsonSerialize(httpPort,                        "I_httpPort",                        j, false);
        JsonSerialize(httpsPort,                       "J_httpsPort",                       j, false);
        JsonSerialize(rtspPort,                        "K_rtspPort",                        j, false);
        JsonSerialize(chaNum,                          "L_chaNum",                          j, false);
        JsonSerialize(state,                           "M_state",                           j, false);
        JsonSerialize(streamNum,                       "N_streamNum",                       j, false);
    }
    
    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(timeStamp,                     "A_timeStamp",                       j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum,                        "B_seqNum",                          j, noUse_isEmptyFlag);
        JsonDeserialize(deviceID,                      "C_deviceID",                        j, noUse_isEmptyFlag);
        JsonDeserialize(longitude,                     "D_longitude",                       j, noUse_isEmptyFlag);
        JsonDeserialize(latitude,                      "E_latitude",                        j, noUse_isEmptyFlag);
        JsonDeserialize(manuFacturer,                 "F_manuFacturer",                   j, noUse_isEmptyFlag);
        JsonDeserialize(model,                         "G_model",                           j, noUse_isEmptyFlag);
        JsonDeserialize(swVersion,                     "H_swVersion",                       j, noUse_isEmptyFlag);
        JsonDeserialize(httpPort,                      "I_httpPort",                        j, noUse_isEmptyFlag);
        JsonDeserialize(httpsPort,                     "J_httpsPort",                       j, noUse_isEmptyFlag);
        JsonDeserialize(rtspPort,                      "K_rtspPort",                        j, noUse_isEmptyFlag);
        JsonDeserialize(chaNum,                        "L_chaNum",                          j, noUse_isEmptyFlag);
        JsonDeserialize(state,                         "M_state",                           j, noUse_isEmptyFlag);
        JsonDeserialize(streamNum,                     "N_streamNum",                       j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timeStamp: "          << timeStamp          << std::endl;
        ss << std::left << std::setw(40) << "seqNum: "             << seqNum             << std::endl;
        ss << std::left << std::setw(40) << "deviceID: "           << deviceID           << std::endl;
        ss << std::left << std::setw(40) << "longitude: "          << longitude          << std::endl;
        ss << std::left << std::setw(40) << "latitude: "           << latitude           << std::endl;
        ss << std::left << std::setw(40) << "manuFacturer: "       << manuFacturer       << std::endl;
        ss << std::left << std::setw(40) << "model: "              << model              << std::endl;
        ss << std::left << std::setw(40) << "swVersion: "          << swVersion          << std::endl;
        ss << std::left << std::setw(40) << "httpPort: "           << httpPort           << std::endl;
        ss << std::left << std::setw(40) << "httpsPort: "          << httpsPort          << std::endl;
        ss << std::left << std::setw(40) << "rtspPort: "           << rtspPort           << std::endl;
        ss << std::left << std::setw(40) << "chaNum: "             << chaNum             << std::endl;
        ss << std::left << std::setw(40) << "state: "              << state              << std::endl;
        ss << std::left << std::setw(40) << "streamNum: "          << streamNum          << std::endl;
        return ss.str();
    }
};
struct RadarRegister : public afl::base::SerializableData
{
    uint64_t timeStamp;             // 时间戳，UTC时间，单位为毫秒
    std::string seqNum;              // 会话唯一标识
    std::string deviceID;            // 设备唯一标识码
    std::string pointNo;             // 点位编号
    std::string pointName;           // 点位名称
    double longitude;                 // 02坐标系的经度
    double latitude;                  // 02坐标系的维度
    double altitude;                  // 海拔高度,单位：m
    double height;                    // 安装高度,单位：m
    double azimuth;                   // 正北偏转角（顺时针）,单位：°
    double pitchAngle;                // 俯仰角，单位：°
    std::string detectionDirection;   // 检测方向：

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(timeStamp,                     "A_timeStamp",                     j, false);
        JsonSerialize(seqNum,                        "B_seqNum",                        j, false);
        JsonSerialize(deviceID,                      "C_deviceID",                      j, false);
        JsonSerialize(pointNo,                       "D_pointNo",                       j, false);
        JsonSerialize(pointName,                     "E_pointName",                     j, false);
        JsonSerialize(longitude,                     "F_longitude",                     j, false);
        JsonSerialize(latitude,                      "G_latitude",                      j, false);
        JsonSerialize(altitude,                      "H_altitude",                      j, false);
        JsonSerialize(height,                        "I_height",                        j, false);
        JsonSerialize(azimuth,                       "J_azimuth",                       j, false);
        JsonSerialize(pitchAngle,                    "K_pitchAngle",                    j, false);
        JsonSerialize(detectionDirection,            "L_detectionDirection",            j, false);
    }
    
    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(timeStamp,                     "A_timeStamp",                     j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum,                        "B_seqNum",                        j, noUse_isEmptyFlag);
        JsonDeserialize(deviceID,                      "C_deviceID",                      j, noUse_isEmptyFlag);
        JsonDeserialize(pointNo,                       "D_pointNo",                       j, noUse_isEmptyFlag);
        JsonDeserialize(pointName,                     "E_pointName",                     j, noUse_isEmptyFlag);
        JsonDeserialize(longitude,                     "F_longitude",                     j, noUse_isEmptyFlag);
        JsonDeserialize(latitude,                      "G_latitude",                      j, noUse_isEmptyFlag);
        JsonDeserialize(altitude,                      "H_altitude",                      j, noUse_isEmptyFlag);
        JsonDeserialize(height,                        "I_height",                        j, noUse_isEmptyFlag);
        JsonDeserialize(azimuth,                       "J_azimuth",                       j, noUse_isEmptyFlag);
        JsonDeserialize(pitchAngle,                    "K_pitchAngle",                    j, noUse_isEmptyFlag);
        JsonDeserialize(detectionDirection,            "L_detectionDirection",            j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timeStamp: "             << timeStamp             << std::endl;
        ss << std::left << std::setw(40) << "seqNum: "                << seqNum                << std::endl;
        ss << std::left << std::setw(40) << "deviceID: "              << deviceID              << std::endl;
        ss << std::left << std::setw(40) << "pointNo: "               << pointNo               << std::endl;
        ss << std::left << std::setw(40) << "pointName: "             << pointName             << std::endl;
        ss << std::left << std::setw(40) << "longitude: "             << longitude             << std::endl;
        ss << std::left << std::setw(40) << "latitude: "              << latitude              << std::endl;
        ss << std::left << std::setw(40) << "altitude: "              << altitude              << std::endl;
        ss << std::left << std::setw(40) << "height: "                << height                << std::endl;
        ss << std::left << std::setw(40) << "azimuth: "               << azimuth               << std::endl;
        ss << std::left << std::setw(40) << "pitchAngle: "            << pitchAngle            << std::endl;
        ss << std::left << std::setw(40) << "detectionDirection: "    << detectionDirection    << std::endl;
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_DEVICE_STATUS
#endif //AIROS2_0_DATA_SYSCHRONIZE_H
