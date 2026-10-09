/*********************************************************************************
 * @file		device_management_base_info_data.h
 * @brief		device_management_base_info_data belongs to CICTCI
 * @details		
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-16
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-4-16 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_BASIC_INFO_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_BASIC_INFO_H

#include "middleware/protocol/om_common/data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_MEC

using namespace os::v2x::protocol::om::common;
// 定义网络协议的枚举体
enum TransProtocolType {
    TRANS_PROTOCOL_HTTP,      // 超文本传输协议
    TRANS_PROTOCOL_HTTPS,     // 安全超文本传输协议
    TRANS_PROTOCOL_FTP,       // 文件传输协议
    TRANS_PROTOCOL_SFTP,      // 安全文件传输协议
    TRANS_PROTOCOL_UDP,       // 用户数据报协议
    TRANS_PROTOCOL_MQTT,      // 消息队列遥测传输协议
    TRANS_PROTOCOL_OTHER      // 其他协议
};

struct RSUData : public afl::base::SerializableData
{
    std::string     rsuSn = STR_DEFAULT_VALUE;
    OmDeviceTypeEnum    deviceType = OM_DEVICE_TYPE_RSU;
    std::string     version = STR_DEFAULT_VALUE;
    bool            versionEmpty = false;
    double          latitude = NUM_DEFAULT_VALUE;
    bool            latitudeEmpty = false;
    double          longitude = NUM_DEFAULT_VALUE;
    bool            longitudeEmpty = false;
    double          elevation = NUM_DEFAULT_VALUE;
    bool            elevationEmpty = false;
    std::string     crossId = STR_DEFAULT_VALUE;
    bool            crossIdEmpty = false;
    int             crossType = NUM_DEFAULT_VALUE;
    bool            crossTypeEmpty = false;
    std::string     crossName = STR_DEFAULT_VALUE;
    bool            crossNameEmpty = false;
    std::string     linkName = STR_DEFAULT_VALUE;
    bool            linkNameEmpty = false;
    int             linkId;
    bool            linkIdEmpty = false;
    json            config = {};
    bool            configEmpty = false;

    virtual void serialize(json& j) override
    {
        JsonSerialize(rsuSn, 	"rsuSn", 	j, false);
        JsonSerialize(deviceType, 	"deviceType", 	j, false);
        JsonSerialize(version, 	"version", 	j, versionEmpty);
        JsonSerialize(latitude, 	"latitude", 	j, latitudeEmpty);
        JsonSerialize(longitude, 	"longitude", 	j, longitudeEmpty);
        JsonSerialize(elevation, 	"elevation", 	j, elevationEmpty);
        JsonSerialize(crossId, 	"crossId", 	j, crossIdEmpty);
        JsonSerialize(crossType, 	"crossType", 	j, crossTypeEmpty);
        JsonSerialize(crossName, 	"crossName", 	j, crossNameEmpty);
        JsonSerialize(linkName, 	"linkName", 	j, linkNameEmpty);
        JsonSerialize(linkId, 	"linkId", 	j, linkIdEmpty);
        JsonSerialize(config, 	"config", 	j, configEmpty);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(rsuSn, 	"rsuSn", 	j, noUse_isEmptyFlag);
        JsonDeserialize(deviceType, 	"deviceType", 	j, noUse_isEmptyFlag);
        JsonDeserialize(version, 	"version", 	j, versionEmpty);
        JsonDeserialize(latitude, 	"latitude", 	j, latitudeEmpty);
        JsonDeserialize(longitude, 	"longitude", 	j, longitudeEmpty);
        JsonDeserialize(elevation, 	"elevation", 	j, elevationEmpty);
        JsonDeserialize(crossId, 	"crossId", 	j, crossIdEmpty);
        JsonDeserialize(crossType, 	"crossType", 	j, crossTypeEmpty);
        JsonDeserialize(crossName, 	"crossName", 	j, crossNameEmpty);
        JsonDeserialize(linkName, 	"linkName", 	j, linkNameEmpty);
        JsonDeserialize(linkId, 	"linkId", 	j, linkIdEmpty);
        JsonDeserialize(config, 	"config", 	j, configEmpty);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "[rsuSn]" << rsuSn <<  std::endl;
        ss << std::left << std::setw(40) << "[deviceType]" << deviceType <<  std::endl;
        ss << std::left << std::setw(40) << "[version]" << version <<  std::endl;
        ss << std::left << std::setw(40) << "[latitude] " << latitude <<  std::endl;
        ss << std::left << std::setw(40) << "[longitude]" << longitude <<  std::endl;
        ss << std::left << std::setw(40) << "[elevation]" << elevation <<  std::endl;
        ss << std::left << std::setw(40) << "[crossId]" << crossId <<  std::endl;
        ss << std::left << std::setw(40) << "[crossType]" << crossType <<  std::endl;
        ss << std::left << std::setw(40) << "[crossName]" << crossName <<  std::endl;
        ss << std::left << std::setw(40) << "[linkName]" << linkName <<  std::endl;
        ss << std::left << std::setw(40) << "[linkId]" << linkId <<  std::endl;
        ss << std::left << std::setw(40) << "[config]" << config <<  std::endl;

        return ss.str();
    }
};

struct SensorData : public afl::base::SerializableData
{
    std::string     sensorSn;
    OmDeviceTypeEnum    deviceType;
    double          latitude = NUM_DEFAULT_VALUE;
    bool            latitudeEmpty = false;

    double          longitude = NUM_DEFAULT_VALUE;
    bool            longitudeEmpty = false;

    double          elevation = NUM_DEFAULT_VALUE;
    bool            elevationEmpty = false;

    std::string     crossId = STR_DEFAULT_VALUE;
    bool            crossIdEmpty = false;

    int             crossType = NUM_DEFAULT_VALUE;
    bool            crossTypeEmpty = false;

    std::string     crossName = STR_DEFAULT_VALUE;
    bool            crossNameEmpty = false;

    std::string     linkName = STR_DEFAULT_VALUE;
    bool            linkNameEmpty = false;

    int             linkId = NUM_DEFAULT_VALUE;
    bool            linkIdEmpty = false;

    virtual void serialize(json& j) override
    {
        JsonSerialize(sensorSn, 	"sensorSn", 	j, false);
        JsonSerialize(deviceType, 	"deviceType", 	j, false);
        JsonSerialize(latitude, 	"latitude", 	j, latitudeEmpty);
        JsonSerialize(longitude, 	"longitude", 	j, longitudeEmpty);
        JsonSerialize(elevation, 	"elevation", 	j, elevationEmpty);
        JsonSerialize(crossId, 	"crossId", 	j, crossIdEmpty);
        JsonSerialize(crossType, 	"crossType", 	j, crossTypeEmpty);
        JsonSerialize(crossName, 	"crossName", 	j, crossNameEmpty);
        JsonSerialize(linkName, 	"linkName", 	j, linkNameEmpty);
        JsonSerialize(linkId, 	"linkId", 	j, linkIdEmpty);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(sensorSn, 	"sensorSn", 	j, noUse_isEmptyFlag);
        JsonDeserialize(deviceType, 	"deviceType", 	j, noUse_isEmptyFlag);
        JsonDeserialize(latitude, 	"latitude", 	j, latitudeEmpty);
        JsonDeserialize(longitude, 	"longitude", 	j, longitudeEmpty);
        JsonDeserialize(elevation, 	"elevation", 	j, elevationEmpty);
        JsonDeserialize(crossId, 	"crossId", 	j, crossIdEmpty);
        JsonDeserialize(crossType, 	"crossType", 	j, crossTypeEmpty);
        JsonDeserialize(crossName, 	"crossName", 	j, crossNameEmpty);
        JsonDeserialize(linkName, 	"linkName", 	j, linkNameEmpty);
        JsonDeserialize(linkId, 	"linkId", 	j, linkIdEmpty);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "[sensorSn]" << sensorSn <<  std::endl;
        ss << std::left << std::setw(40) << "[deviceType]" << deviceType <<  std::endl;
        ss << std::left << std::setw(40) << "[latitude]" << (latitudeEmpty ? "Empty" : std::to_string(latitude)) <<  std::endl;
        ss << std::left << std::setw(40) << "[longitude]" << longitude <<  std::endl;
        ss << std::left << std::setw(40) << "[elevation]" << elevation <<  std::endl;
        ss << std::left << std::setw(40) << "[crossId]" << crossId <<  std::endl;
        ss << std::left << std::setw(40) << "[crossType]" << crossType <<  std::endl;
        ss << std::left << std::setw(40) << "[crossName]" << crossName <<  std::endl;
        ss << std::left << std::setw(40) << "[linkName]" << linkName <<  std::endl;
        ss << std::left << std::setw(40) << "[linkId]" << linkId <<  std::endl;

        return ss.str();
    }
};
struct DeviceBasicInfoData : public afl::base::SerializableData
{
    uint64_t    timeStamp;
    std::string seqNum;
    std::string rscuEsn;

    std::string regionId = STR_DEFAULT_VALUE;
    bool regionIdEmpty = false;

    int         roadId = NUM_DEFAULT_VALUE;
    bool roadIdEmpty = false;

    std::string roadName = STR_DEFAULT_VALUE;
    bool roadNameEmpty = false;

    int         roadType = NUM_DEFAULT_VALUE;
    bool roadTypeEmpty = false;

    std::string crossId = STR_DEFAULT_VALUE;
    bool crossIdEmpty = false;

    int         crossType = NUM_DEFAULT_VALUE;
    bool crossTypeEmpty = false;

    std::string crossName = STR_DEFAULT_VALUE;
    bool crossNameEmpty = false;

    int         linkId = NUM_DEFAULT_VALUE;
    bool linkIdEmpty = false;

    std::string linkName = STR_DEFAULT_VALUE;
    bool linkNameEmpty = false;


    double      latitude;
    bool latitudeEmpty = false;


    double      longitude;
    bool longitudeEmpty = false;

    double      elevation = NUM_DEFAULT_VALUE;
    bool elevationEmpty = false;

    OmDeviceTypeEnum  deviceType;

    std::string supplier = STR_DEFAULT_VALUE;
    bool supplierEmpty = false;

    std::string owner = STR_DEFAULT_VALUE;
    bool ownerEmpty = false;

    std::string protocolVersion = PROTOCOL_VERSION;
    bool protocolVersionEmpty = false;

    std::string imei = STR_DEFAULT_VALUE;
    bool imeiEmpty = false;

    std::string iccId = STR_DEFAULT_VALUE;
    bool iccIdEmpty = false;

    RunStatusEnum         rscuStatus;
    bool rscuStatusEmpty = false;

    NetworkStatusEnum         active;

    TransProtocolType         transProtocal = TRANS_PROTOCOL_MQTT;
    bool transProtocalEmpty = false;

    std::string softwareVersion;
    bool softwareVersionEmpty = false;

    std::string hardwareVersion;
    bool hardwareVersionEmpty = false;

    int         rsuNum = 0;
    bool rsuNumEmpty = false;

    std::vector<RSUData> rsulist;
    bool rsulistEmpty = false;

    int         sensorNum = 0;
    bool sensorNumEmpty = false;

    std::vector<SensorData> sensorlist;
    bool sensorlistEmpty = false;

    bool ack = false;
    bool ackEmpty = false;

    std::string apiDataVer = "v3.4"; // 运维3.4xinzeng
    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, 	"timestamp", 	j, false);
        JsonSerialize(seqNum, 	"seqNum", 	j, false);
        JsonSerialize(rscuEsn, 	"rscuEsn", 	j, false);
        JsonSerialize(regionId, 	"regionId", 	j, regionIdEmpty);
        JsonSerialize(roadId, 	"roadId", 	j, roadIdEmpty);
        JsonSerialize(roadName, 	"roadName", 	j, roadNameEmpty);
        JsonSerialize(roadType, 	"roadType", 	j, roadTypeEmpty );
        JsonSerialize(crossId, 	"crossId", 	j, crossIdEmpty);
        JsonSerialize(crossType, 	"crossType", 	j, crossTypeEmpty );
        JsonSerialize(crossName, 	"crossName", 	j, crossNameEmpty);
        JsonSerialize(linkId, 	"linkId", 	j, linkIdEmpty );
        JsonSerialize(linkName, 	"linkName", 	j, linkNameEmpty);
        JsonSerialize(latitude, 	"latitude", 	j, latitudeEmpty );
        JsonSerialize(longitude, 	"longitude", 	j, longitudeEmpty);
        JsonSerialize(elevation, 	"elevation", 	j, elevationEmpty);
        JsonSerialize(deviceType, 	"deviceType", 	j, false);
        JsonSerialize(supplier, 	"supplier", 	j, supplierEmpty);
        JsonSerialize(owner, 	"owner", 	j, ownerEmpty );
        JsonSerialize(protocolVersion, 	"protocolVersion", 	j, protocolVersionEmpty );
        JsonSerialize(imei, 	"imei", 	j, imeiEmpty);
        JsonSerialize(iccId, 	"iccId", 	j, iccIdEmpty);
        JsonSerialize(rscuStatus, 	"rscuStatus", 	j, rscuStatusEmpty);
        JsonSerialize(active, 	"active", 	j, false );
        JsonSerialize(transProtocal, 	"transProtocal", 	j, transProtocalEmpty );
        JsonSerialize(softwareVersion, 	"softwareVersion", 	j, softwareVersionEmpty );
        JsonSerialize(hardwareVersion, 	"hardwareVersion", 	j, hardwareVersionEmpty );
        JsonSerialize(rsuNum, 	"rsuNum", 	j, rsuNumEmpty);
        JsonSerialize(rsulist, 	"rsulist", 	j, rsulistEmpty);
        JsonSerialize(sensorNum, 	"sensorNum", 	j, sensorNumEmpty );
        JsonSerialize(sensorlist, 	"sensorlist", 	j, sensorlistEmpty);
        JsonSerialize(ack, 	"ack", 	j, ackEmpty);
        JsonSerialize(apiDataVer, 	"apiDataVer", 	j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, 	"timestamp", 	j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, 	"seqNum", 	j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, 	"rscuEsn", 	j, noUse_isEmptyFlag);
        JsonDeserialize(regionId, 	"regionId", 	j, regionIdEmpty);
        JsonDeserialize(roadId, 	"roadId", 	j, roadIdEmpty);
        JsonDeserialize(roadName, 	"roadName", 	j, roadNameEmpty);
        JsonDeserialize(roadType, 	"roadType", 	j, roadTypeEmpty );
        JsonDeserialize(crossId, 	"crossId", 	j, crossIdEmpty);
        JsonDeserialize(crossType, 	"crossType", 	j, crossTypeEmpty );
        JsonDeserialize(crossName, 	"crossName", 	j, crossNameEmpty);
        JsonDeserialize(linkId, 	"linkId", 	j, linkIdEmpty );
        JsonDeserialize(linkName, 	"linkName", 	j, linkNameEmpty);
        JsonDeserialize(latitude, 	"latitude", 	j, latitudeEmpty );
        JsonDeserialize(longitude, 	"longitude", 	j, longitudeEmpty);
        JsonDeserialize(elevation, 	"elevation", 	j, elevationEmpty);
        JsonDeserialize(deviceType, 	"deviceType", 	j, noUse_isEmptyFlag);
        JsonDeserialize(supplier, 	"supplier", 	j, supplierEmpty);
        JsonDeserialize(owner, 	"owner", 	j, ownerEmpty );
        JsonDeserialize(protocolVersion, 	"protocolVersion", 	j, protocolVersionEmpty );
        JsonDeserialize(imei, 	"imei", 	j, imeiEmpty);
        JsonDeserialize(iccId, 	"iccId", 	j, iccIdEmpty);
        JsonDeserialize(rscuStatus, 	"rscuStatus", 	j, rscuStatusEmpty);
        JsonDeserialize(active, 	"active", 	j, noUse_isEmptyFlag );
        JsonDeserialize(transProtocal, 	"transProtocal", 	j, transProtocalEmpty );
        JsonDeserialize(softwareVersion, 	"softwareVersion", 	j, softwareVersionEmpty );
        JsonDeserialize(hardwareVersion, 	"hardwareVersion", 	j, hardwareVersionEmpty );
        JsonDeserialize(rsuNum, 	"rsuNum", 	j, rsuNumEmpty);
        JsonDeserialize(rsulist, 	"rsulist", 	j, rsulistEmpty);
        JsonDeserialize(sensorNum, 	"sensorNum", 	j, sensorNumEmpty );
        JsonDeserialize(sensorlist, 	"sensorlist", 	j, sensorlistEmpty);
        JsonDeserialize(ack, 	"ack", 	j, ackEmpty);

        JsonDeserialize(apiDataVer, 	"apiDataVer", 	j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "timestamp: " << timeStamp <<  std::endl;
        ss << std::left << std::setw(40) << "seqNum: " << seqNum <<  std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: " << rscuEsn <<  std::endl;
        ss << std::left << std::setw(40) << "regionId: " << regionId <<  std::endl;
        ss << std::left << std::setw(40) << "roadId: " << roadId <<  std::endl;
        ss << std::left << std::setw(40) << "roadName: " << roadName <<  std::endl;
        ss << std::left << std::setw(40) << "roadType: " << roadType <<  std::endl;
        ss << std::left << std::setw(40) << "crossId: " << crossId <<  std::endl;
        ss << std::left << std::setw(40) << "crossType: " << crossType <<  std::endl;
        ss << std::left << std::setw(40) << "crossName: " << crossName <<  std::endl;
        ss << std::left << std::setw(40) << "linkId: " << linkId <<  std::endl;
        ss << std::left << std::setw(40) << "linkName: " << linkName <<  std::endl;
        ss << std::left << std::setw(40) << "latitude: " << latitude <<  std::endl;
        ss << std::left << std::setw(40) << "longitude: " << longitude <<  std::endl;
        ss << std::left << std::setw(40) << "elevation: " << elevation <<  std::endl;
        ss << std::left << std::setw(40) << "deviceType: " << deviceType <<  std::endl;
        ss << std::left << std::setw(40) << "supplier: " << supplier <<  std::endl;
        ss << std::left << std::setw(40) << "owner: " << owner <<  std::endl;
        ss << std::left << std::setw(40) << "protocolVersion: " << protocolVersion <<  std::endl;
        ss << std::left << std::setw(40) << "imei: " << imei <<  std::endl;
        ss << std::left << std::setw(40) << "iccId: " << iccId <<  std::endl;
        ss << std::left << std::setw(40) << "rscuStatus: " << rscuStatus <<  std::endl;
        ss << std::left << std::setw(40) << "active: " << active <<  std::endl;
        ss << std::left << std::setw(40) << "transProtocal: " << transProtocal <<  std::endl;
        ss << std::left << std::setw(40) << "softwareVersion: " << softwareVersion <<  std::endl;
        ss << std::left << std::setw(40) << "hardwareVersion: " << hardwareVersion <<  std::endl;
        ss << std::left << std::setw(40) << "rsuNum: " << rsuNum <<  std::endl;
        ss << std::left << std::setw(40) << "rsulist: " <<  std::endl;
        for (const auto& rsu : rsulist) {
            ss << rsu.to_string() <<  std::endl;
        }
        ss << std::left << std::setw(40) << "sensorNum: " << sensorNum <<  std::endl;
        ss << std::left << std::setw(40) << "sensorlist: " <<  std::endl;
        for (const auto& sensor : sensorlist) {
            ss << sensor.to_string() <<  std::endl;
        }
        ss << std::left << std::setw(40) << "ack: " << (ack ? "True" : "False") <<  std::endl;
        return ss.str();
    }
};
enum DeviceBasicInfoAckStatus
{
    FAILURE_REGISTER = 0 ,
    SUCESS_REGISTER = 1
};
struct DeviceBasicInfoAckData : public afl::base::SerializableData
{
    uint64_t    timeStamp;
    std::string seqNum;
    std::string rscuEsn;

    DeviceBasicInfoAckStatus status;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(timeStamp,          "timestamp",          j, false);
        JsonSerialize(seqNum,             "seqNum",             j, false);
        JsonSerialize(rscuEsn,             "rscuEsn",             j, false);
        JsonSerialize(status,              "status",              j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(timeStamp,          "timestamp",          j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum,             "seqNum",             j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn,             "rscuEsn",             j, noUse_isEmptyFlag);
        JsonDeserialize(status,              "status",              j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "TimeStamp: " << timeStamp <<  std::endl;
        ss << std::left << std::setw(40) << "SeqNum: " << seqNum <<  std::endl;
        ss << std::left << std::setw(40) << "rscuESn: " << rscuEsn <<  std::endl;
        ss << std::left << std::setw(40) << "Status: " << status <<  std::endl;

        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif
