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

#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_OTA_DATA_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_OTA_DATA_H
#include "middleware/protocol/om_common/data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_MEC
using namespace os::v2x::protocol::om::common;
enum OTA_TYPE{OTA_HTTPS = 1};
struct OTAUpdateDownData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string protocolVersion;
    std::string updateVersion;
    std::string downloadUrl;
    std::string downloadChkPara;
    uint64_t updateTime;
    std::string token;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(protocolVersion, "protocolVersion", j, false);
        JsonSerialize(updateVersion, "updateVersion", j, false);
        JsonSerialize(downloadUrl, "downloadUrl", j, false);
        JsonSerialize(downloadChkPara, "downloadChkPara", j, false);
        JsonSerialize(updateTime, "updateTime", j, false);
        JsonSerialize(token, "token", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(protocolVersion, "protocolVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(updateVersion, "updateVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(downloadUrl, "downloadUrl", j, noUse_isEmptyFlag);
        JsonDeserialize(downloadChkPara, "downloadChkPara", j, noUse_isEmptyFlag);
        JsonDeserialize(updateTime, "updateTime", j, noUse_isEmptyFlag);
        JsonDeserialize(token, "token", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "timeStamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "seqNum: " << seqNum << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "protocolVersion: " << protocolVersion << std::endl;
        ss << std::left << std::setw(40) << "updateVersion: " << updateVersion << std::endl;
        ss << std::left << std::setw(40) << "downloadUrl: " << downloadUrl << std::endl;
        ss << std::left << std::setw(40) << "downloadChkPara: " << downloadChkPara << std::endl;
        ss << std::left << std::setw(40) << "updateTime: " << updateTime << std::endl;
        ss << std::left << std::setw(40) << "token: " << token << std::endl;

        return ss.str();
    }
};

enum CodeType {
    SUCCESS = 0,         // 0: 处理成功
    FAILURE = 1,         // 1: 处理失败
    ALREADY_UP_TO_DATE = 2, // 2: 已经是最新版本
    OTHER_FAILURE = 3      // 3: 其他故障
};
struct OTAUpdateStatusUpData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    CodeType code;
    std::string softwareVersion;
    std::string hardwareVersion;
    std::string description;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(code, "code", j, false);
        JsonSerialize(softwareVersion, "softwareVersion", j, false);
        JsonSerialize(hardwareVersion, "hardwareVersion", j, false);
        JsonSerialize(description, "description", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(code, "code", j, noUse_isEmptyFlag);
        JsonDeserialize(softwareVersion, "softwareVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(hardwareVersion, "hardwareVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(description, "description", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(20) << "Time Stamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(20) << "Seq Num: " << seqNum << std::endl;
        ss << std::left << std::setw(20) << "RSCU ESn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(20) << "Code: " << code << std::endl;
        ss << std::left << std::setw(20) << "Software Version: " << softwareVersion << std::endl;
        ss << std::left << std::setw(20) << "Hardware Version: " << hardwareVersion << std::endl;
        ss << std::left << std::setw(20) << "Description: " << description << std::endl;
        return ss.str();
    }
};

struct DeviceVersionQueryAckData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string protocolVersion;
    std::string softwareVersion;
    std::string hardwareVersion;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(protocolVersion, "protocolVersion", j, false);
        JsonSerialize(softwareVersion, "softwareVersion", j, false);
        JsonSerialize(hardwareVersion, "hardwareVersion", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(protocolVersion, "protocolVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(softwareVersion, "softwareVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(hardwareVersion, "hardwareVersion", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "timeStamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "seqNum: " << seqNum << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "protocolVersion: " << protocolVersion << std::endl;
        ss << std::left << std::setw(40) << "softwareVersion: " << softwareVersion << std::endl;
        ss << std::left << std::setw(40) << "hardwareVersion: " << hardwareVersion << std::endl;

        return ss.str();
    }
};
//取消升级
struct OtaUpdateCancelData : public afl::base::SerializableData
{
    long        timestamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string updateVersion;
    int         action;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(timestamp,                       "timestamp",                       j, false);
        JsonSerialize(seqNum,                          "seqNum",                          j, false);
        JsonSerialize(rscuEsn,                         "rscuEsn",                         j, false);
        JsonSerialize(updateVersion,                   "updateVersion",                   j, false);
        JsonSerialize(action,                          "action",                          j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(timestamp,                     "timestamp",                       j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum,                        "seqNum",                          j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn,                       "rscuEsn",                         j, noUse_isEmptyFlag);
        JsonDeserialize(updateVersion,                 "updateVersion",                   j, noUse_isEmptyFlag);
        JsonDeserialize(action,                        "action",                          j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timestamp: "            << timestamp            << std::endl;
        ss << std::left << std::setw(40) << "seqNum: "               << seqNum               << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: "              << rscuEsn              << std::endl;
        ss << std::left << std::setw(40) << "updateVersion: "        << updateVersion        << std::endl;
        ss << std::left << std::setw(40) << "action: "               << action               << std::endl;
        return ss.str();
    }
};

//取消升级反馈
struct OtaUpdateCancelAckData : public afl::base::SerializableData
{
    long        timestamp;
    std::string seqNum;
    std::string deviceID;
    std::string state;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(timestamp,                       "timestamp",                       j, false);
        JsonSerialize(seqNum,                          "seqNum",                          j, false);
        JsonSerialize(deviceID,                         "deviceID",                         j, false);
        JsonSerialize(state,                   "state",                   j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(timestamp,                     "timestamp",                       j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum,                        "seqNum",                          j, noUse_isEmptyFlag);
        JsonDeserialize(deviceID,                       "deviceID",                         j, noUse_isEmptyFlag);
        JsonDeserialize(state,                 "state",                   j, noUse_isEmptyFlag);
    }

};

NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif
