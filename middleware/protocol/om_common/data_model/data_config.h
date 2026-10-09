/*********************************************************************************
 * @file		config_management_data.h
 * @brief		config_management_data belongs to CICTCI
 * @details		
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-15
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-4-15 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef MQTT_CLIENT_MEC_OM_CONFIG_DATA_H
#define MQTT_CLIENT_MEC_OM_CONFIG_DATA_H
#include "data_common.h"
NAMESPACE_START_OM_COMPONENT_COMMON

//配置查询
struct ConfigQueryData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string actionName = "baseInfoEnquire";

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, 	"timestamp", 	j, false);
        JsonSerialize(seqNum, 	"seqNum", 	j, false);
        JsonSerialize(rscuEsn, 	"rscuEsn", 	j, false);
        JsonSerialize(actionName, 	"actionName", 	j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, 	"timestamp", 	j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, 	"seqNum", 	j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, 	"rscuEsn", 	j, noUse_isEmptyFlag);
        JsonDeserialize(actionName, 	"actionName", 	j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "timeStamp :" << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "seqNum :" << seqNum << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn :" << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "actionName :" << actionName << std::endl;
        return ss.str();
    }
};

//配置查询响应
struct ConfigQueryAckData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string pointNo;
    std::string pointName;
    double longitude;
    double latitude;
    double altitude;
    double height;
    SiteTypeEnum siteType;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, 	"timestamp", 	j, false);
        JsonSerialize(seqNum, 	"seqNum", 	j, false);
        JsonSerialize(rscuEsn, 	"rscuEsn", 	j, false);
        JsonSerialize(pointNo, 	"pointNo", 	j, false);
        JsonSerialize(pointName, 	"pointName", 	j, false);
        JsonSerialize(longitude, 	"longitude", 	j, false);
        JsonSerialize(latitude, 	"latitude", 	j, false);
        JsonSerialize(altitude, 	"altitude", 	j, false);

        JsonSerialize(height, 	"height", 	j, false);
        JsonSerialize(siteType, 	"siteType", 	j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, 	"timestamp", 	j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, 	"seqNum", 	j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, 	"rscuEsn", 	j, noUse_isEmptyFlag);
        JsonDeserialize(pointNo, 	"pointNo", 	j, noUse_isEmptyFlag);
        JsonDeserialize(pointName, 	"pointName", 	j, noUse_isEmptyFlag);
        JsonDeserialize(longitude, 	"longitude", 	j, noUse_isEmptyFlag);
        JsonDeserialize(latitude, 	"latitude", 	j, noUse_isEmptyFlag);
        JsonDeserialize(altitude, 	"altitude", 	j, noUse_isEmptyFlag);

        JsonDeserialize(height, 	"height", 	j, noUse_isEmptyFlag);
        JsonDeserialize(siteType, 	"siteType", 	j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Time Stamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "Seq Num: " << seqNum << std::endl;
        ss << std::left << std::setw(40) << "RSCU ESNum: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "Point No: " << pointNo << std::endl;
        ss << std::left << std::setw(40) << "Point Name: " << pointName << std::endl;
        ss << std::left << std::setw(40) << "Longitude: " << longitude << std::endl;
        ss << std::left << std::setw(40) << "Latitude: " << latitude << std::endl;
        ss << std::left << std::setw(40) << "Altitude: " << altitude << std::endl;
        ss << std::left << std::setw(40) << "Height: " << height << std::endl;
        ss << std::left << std::setw(40) << "Site Type: " << siteType << std::endl;

        return ss.str();
    }
};
///////////////////////////////////////////////////////////////////////////////


struct ConfigUpdateData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string actionName = "baseInfoEnquire";
    std::string protocolVersion = PROTOCOL_VERSION;
    int runningInfoRate;
    int heartRate;
    string addressChg;
    std::string addressIP;
    std::string netMask;
    std::string gateway;
    LogLevelEnum logLevel;
    uint64_t time;
    PowerEnum power;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(actionName, "actionName", j, false);
        JsonSerialize(protocolVersion, "protocolVersion", j, false);
        JsonSerialize(runningInfoRate, "runningInfoRate", j, false);
        JsonSerialize(heartRate, "heartRate", j, false);
        JsonSerialize(addressChg, "addressChg", j, false);
        JsonSerialize(addressIP, "addressIP", j, false);
        JsonSerialize(netMask, "netMask", j, false);
        JsonSerialize(gateway, "gateway", j, false);
        JsonSerialize(logLevel, "logLevel", j, false);
        JsonSerialize(time, "time", j, false);
        JsonSerialize(power, "power", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(actionName, "actionName", j, noUse_isEmptyFlag);
        JsonDeserialize(protocolVersion, "protocolVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(runningInfoRate, "runningInfoRate", j, noUse_isEmptyFlag);
        JsonDeserialize(heartRate, "heartRate", j, noUse_isEmptyFlag);
        JsonDeserialize(addressChg, "addressChg", j, noUse_isEmptyFlag);
        JsonDeserialize(addressIP, "addressIP", j, noUse_isEmptyFlag);
        JsonDeserialize(netMask, "netMask", j, noUse_isEmptyFlag);
        JsonDeserialize(gateway, "gateway", j, noUse_isEmptyFlag);
        JsonDeserialize(logLevel, "logLevel", j, noUse_isEmptyFlag);
        JsonDeserialize(time, "time", j, noUse_isEmptyFlag);
        JsonDeserialize(power, "power", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "timeStamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "seqNum: " << seqNum << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "actionName: " << actionName << std::endl;
        ss << std::left << std::setw(40) << "protocolVersion: " << protocolVersion << std::endl;
        ss << std::left << std::setw(40) << "runningInfoRate: " << runningInfoRate << std::endl;
        ss << std::left << std::setw(40) << "heartRate: " << heartRate << std::endl;
        ss << std::left << std::setw(40) << "addressChg: " << addressChg << std::endl;
        ss << std::left << std::setw(40) << "addressIP: " << addressIP << std::endl;
        ss << std::left << std::setw(40) << "netMask: " << netMask << std::endl;
        ss << std::left << std::setw(40) << "gateway: " << gateway << std::endl;
        ss << std::left << std::setw(40) << "logLevel: " << logLevel << std::endl;
        ss << std::left << std::setw(40) << "time: " << time << std::endl;
        ss << std::left << std::setw(40) << "power: " << power << std::endl;
        return ss.str();
    }
};
struct ConfigUpdateAckData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    StatusEnum status;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(status, "status", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(status, "status", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Time Stamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "Seq Num: " << seqNum << std::endl;
        ss << std::left << std::setw(40) << "RSCU ESn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "Status: " << status << std::endl;
        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_COMMON
#endif //NCS_MEC_CONFIG_MANAGEMENT_DATA_H
