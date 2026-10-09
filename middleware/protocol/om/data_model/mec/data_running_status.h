/*********************************************************************************
 * @file		performence_management_running_status_data.h
 * @brief		performence_management_running_status_data belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-24
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-4-24 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_RUNNING_STATUS_DATA_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_RUNNING_STATUS_DATA_H
#include "middleware/protocol/om_common/data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_MEC
using namespace os::v2x::protocol::om::common;

struct RsuStatusData : public afl::base::SerializableData
{
    std::string rscuEsn;
    int deviceType;
    RunStatusEnum status = RUN_STATUS_FAULT;
    bool statusEmpty = false;
    NetworkStatusEnum active;

    virtual void serialize(json& j) override
    {
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(deviceType, "deviceType", j, false);
        JsonSerialize(status, "status", j, statusEmpty);
        JsonSerialize(active, "active", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceType, "deviceType", j, noUse_isEmptyFlag);
        JsonDeserialize(status, "status", j, statusEmpty);
        JsonDeserialize(active, "active", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "deviceType: " << deviceType << std::endl;
        ss << std::left << std::setw(40) << "status: " << status << std::endl;
        ss << std::left << std::setw(40) << "active: " << active << std::endl;
        return ss.str();
    }
};

struct SensorStatusData : public afl::base::SerializableData
{
    std::string sensorSn;
    int deviceType;
    RunStatusEnum status = RUN_STATUS_FAULT;
    bool statusEmpty = false;
    NetworkStatusEnum active;

    virtual void serialize(json& j) override
    {
        JsonSerialize(sensorSn, "sensorSn", j, false);
        JsonSerialize(deviceType, "deviceType", j, false);
        JsonSerialize(status, "status", j, statusEmpty);
        JsonSerialize(active, "active", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(sensorSn, "sensorSn", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceType, "deviceType", j, noUse_isEmptyFlag);
        JsonDeserialize(status, "status", j, statusEmpty);
        JsonDeserialize(active, "active", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "SensorSn :" << sensorSn << std::endl;
        ss << std::left << std::setw(40) << "DeviceType :" << deviceType << std::endl;
        ss << std::left << std::setw(40) << "Status :" << status << std::endl;
        ss << std::left << std::setw(40) << "StatusEmpty :" << (statusEmpty ? "True" : "False") << std::endl;
        ss << std::left << std::setw(40) << "Active :" << active << std::endl;
        return ss.str();
    }
};
enum FaultType {
    CAMERA_FAILURE = 0,    // 0: 摄像机故障
    MILLIMETER_WAVE_RADAR_FAILURE = 1, // 1: 毫米波雷达故障
    LASER_RADAR_FAILURE = 3, // 3: 激光雷达故障
    // 可以在这里继续添加更多的故障类型
     FAULT_TYPE_OTHER = 4, // 4: ... 添加描述
};
enum DeviceType {
    DEVICE_TYPE_CAMERA = 0,    // 0: 摄像机故障
    DEVICE_TYPE_MILLIMETER_WAVE_RADAR = 1, // 1: 毫米波雷达故障
    DEVICE_TYPE_LASER_RADAR = 3, // 3: 激光雷达故障
    // 可以在这里继续添加更多的故障类型
    DEVICE_TYPE_OTHER = 4, // 4: ... 添加描述
};
struct FaultData : public afl::base::SerializableData
{
    std::string deviceSn;
    DeviceType deviceType;

    FaultType faultType = FAULT_TYPE_OTHER;
    bool faultTypeEmpty = false;

    double faultTime = NUM_DEFAULT_VALUE;
    bool faultTimeEmpty = false;
    std::string faultDescription = STR_DEFAULT_VALUE;
    bool faultDescriptionEmpty = false;

    virtual void serialize(json& j) override
    {
        JsonSerialize(deviceSn, "deviceSn", j, false);
        JsonSerialize(deviceType, "deviceType", j, false);
        JsonSerialize(faultType, "faultType", j, faultTypeEmpty);
        JsonSerialize(faultTime, "faultTime", j, faultTimeEmpty);
        JsonSerialize(faultDescription, "faultDescription", j, faultDescriptionEmpty);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(deviceSn, "deviceSn", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceType, "deviceType", j, noUse_isEmptyFlag);
        JsonDeserialize(faultType, "faultType", j, faultTypeEmpty);
        JsonDeserialize(faultTime, "faultTime", j, faultTimeEmpty);
        JsonDeserialize(faultDescription, "faultDescription", j, faultDescriptionEmpty);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "DeviceSn: " << deviceSn << std::endl;
        ss << std::left << std::setw(40) << "DeviceType: " << deviceType << std::endl;
        ss << std::left << std::setw(40) << "FaultType: " << faultType << (faultTypeEmpty ? " (empty)" : "") << std::endl;
        ss << std::left << std::setw(40) << "FaultTime: " << faultTime << (faultTimeEmpty ? " (empty)" : "") << std::endl;
        ss << std::left << std::setw(40) << "FaultDescription: " << faultDescription << (faultDescriptionEmpty ? " (empty)" : "") << std::endl;
        return ss.str();
    }
};


struct RunningStatusData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string regionId = STR_DEFAULT_VALUE;
    bool regionIdEmpty = false;

    double longitude = NUM_DEFAULT_VALUE;
    bool longitudeEmpty = false;

    double latitude = NUM_DEFAULT_VALUE;
    bool latitudeEmpty = false;

    double elevation = NUM_DEFAULT_VALUE;
    bool elevationEmpty = false;

    RunStatusEnum rscuStatus = RUN_STATUS_NORMAL;
    bool rscuStatusEmpty = false;

    NetworkStatusEnum active = NETWORK_STATUS_ONLINE;

    int rsuNum = 0;
    bool rsuNumEmpty = false;

    std::vector<RsuStatusData> rsuStatusList;
    bool rsuStatusListEmpty = false;

    int sensorNum = 0;
    bool sensorNumEmpty = false;

    std::vector<SensorStatusData> sensorStatusList;
    bool sensorStatusListEmpty = false;

    std::vector<FaultData> faultList;
    bool faultListEmpty = true;

    bool ack;
    bool ackEmpty = true;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(regionId, "regionId", j, regionIdEmpty);
        JsonSerialize(longitude, "longitude", j, longitudeEmpty);
        JsonSerialize(latitude, "latitude", j, latitudeEmpty);
        JsonSerialize(elevation, "elevation", j, elevationEmpty);
        JsonSerialize(rscuStatus, "rscuStatus", j, rscuStatusEmpty);
        JsonSerialize(active, "active", j, false);
        JsonSerialize(rsuNum, "rsuNum", j, rsuNumEmpty);
        JsonSerialize(rsuStatusList, "rsuStatusList", j, rsuStatusListEmpty);
        JsonSerialize(sensorNum, "sensorNum", j, sensorNumEmpty);
        JsonSerialize(sensorStatusList, "sensorStatusList", j, sensorStatusListEmpty);
        JsonSerialize(faultList, "faultList", j, faultListEmpty);
        JsonSerialize(ack, "ack", j, ackEmpty);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(regionId, "regionId", j, regionIdEmpty);
        JsonDeserialize(longitude, "longitude", j, longitudeEmpty);
        JsonDeserialize(latitude, "latitude", j, latitudeEmpty);
        JsonDeserialize(elevation, "elevation", j, elevationEmpty);
        JsonDeserialize(rscuStatus, "rscuStatus", j, rscuStatusEmpty);
        JsonDeserialize(active, "active", j, noUse_isEmptyFlag);
        JsonDeserialize(rsuNum, "rsuNum", j, rsuNumEmpty);
        JsonDeserialize(rsuStatusList, "rsuStatusList", j, rsuStatusListEmpty);
        JsonDeserialize(sensorNum, "sensorNum", j, sensorNumEmpty);
        JsonDeserialize(sensorStatusList, "sensorStatusList", j, sensorStatusListEmpty);
        JsonDeserialize(faultList, "faultList", j, faultListEmpty);
        JsonDeserialize(ack, "ack", j, ackEmpty);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "timeStamp :" << timeStamp << std::endl;
		ss << std::left << std::setw(40) << "seqNum :" << seqNum << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn :" << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "regionId :" << regionId << std::endl;
        ss << std::left << std::setw(40) << "longitude :" << (longitudeEmpty ? "N/A" : std::to_string(longitude)) << std::endl;
        ss << std::left << std::setw(40) << "latitude :" << (latitudeEmpty ? "N/A" : std::to_string(latitude)) << std::endl;
        ss << std::left << std::setw(40) << "elevation :" << (elevationEmpty ? "N/A" : std::to_string(elevation)) << std::endl;
        ss << std::left << std::setw(40) << "rscuStatus :" << (rscuStatusEmpty ? "N/A" : std::to_string(rscuStatus)) << std::endl;
        ss << std::left << std::setw(40) << "active :" << active << std::endl;
        ss << std::left << std::setw(40) << "rsuNum :" << (rsuNumEmpty ? "N/A" : std::to_string(rsuNum)) << std::endl;
        ss << std::left << std::setw(40) << "rsuStatusList:\n";
        for (const auto& rsuStatus : rsuStatusList) {
            ss << rsuStatus.to_string() << std::endl;
        }
        ss << std::left << std::setw(40) << "sensorNum :" << (sensorNumEmpty ? "N/A" : std::to_string(sensorNum)) << std::endl;
        ss << std::left << std::setw(40) << "sensorStatusList :\n";
        for (const auto& sensorStatus : sensorStatusList) {
            ss << sensorStatus.to_string() << std::endl;
        }
        ss << std::left << std::setw(40) << "faultList :\n";
        for (const auto& fault : faultList) {
            ss << fault.to_string() << std::endl;
        }
        ss << std::left << std::setw(40) << "ack :" << (ack ? "True" : "False") << std::endl;
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif
