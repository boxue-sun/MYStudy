/*********************************************************************************
 * @file		configer.h
 * @brief		configer belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-6-9
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-6-9 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_CONFIGER_CONFIGER_OM_MEC_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_CONFIGER_CONFIGER_OM_MEC_H


#include "middleware/protocol//om_common/data_model/data_performence.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "middleware/protocol/om_common/data_model/data_common.h"
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_topic_ccindex.h"
NAMESPACE_START_OM_COMPONENT_MEC
using namespace airos::base::workparam;
using namespace os::v2x::protocol::om::common;
using namespace os::v2x::protocol::om::mec;
using namespace os::v2x::protocol::ccindex;
struct MqttPublishConfigerPeriod : public afl::base::SerializableData
{
public:
    double 	periodHeartBeatUp 		= 60.0;
    double 	periodRunningInfoUp 	= 60.0;
    double 	periodAlarmInfoUp 		= 10.0;
    double  periodDownloadBin        = 5.0;
    double  periodPerformenceUp     = 600;
    double  periodTimingUp     = 60;
    double  periodDeviceVersion  = 3600;
    double  periodAlarmMonitor = 1;
    double  periodOmWorkParamConfigerMonitor = 5;
    double  periodAlarmPublishInterval = 60;
    double  periodPublishCameraInterExterParamInterval = 6;
    double  periodGetCameraInterExterParamInterval = 4;
    double  periodQueryCameraInterExterParamInterval = 180;
    
    double  periodPublishRadarInterExterParamInterval = 6;
    double  periodQueryRadarInterExterParamInterval = 180;
    double  periodPublishTrafficlightDetectDataInterval = 1.1;


    uint32_t publishAlarmDataIntervalUnit = 1000; //秒和毫秒之间的换算单位
    double 	periodMonitorCameraHeartBeatUp 		= 60.0;
    double 	periodMonitorRadarHeartBeatUp 		= 60.0;

    double 	periodMonitorPerformanceData 		= 30.0;

    double 	periodMonitorCameraHeartBeatForAlarm 	= 61.0;
    double 	periodMonitorRadarHeartBeatForAlarm 	= 61.0;
    double  periodMonitorOmStatus 	= 10.0;

    double periodPingSensorDeviceActive = 10;

    double periodCheckMecStatus = 1.0;

    double periodCheckSpatDataStatus = 5;  //信号机数据状态
    double periodSpatDataStatusUploadInterval = 1; //单位s
private:
    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(periodHeartBeatUp, "A_PeriodHeartBeatUp", j, false);
        JsonSerialize(periodRunningInfoUp, "B_PeriodRunningInfoUp", j, false);
        JsonSerialize(periodAlarmInfoUp, "C_PeriodAlarmInfoUp", j, false);
        JsonSerialize(periodDownloadBin, "D_PeriodDownloadBin", j, false);
        JsonSerialize(periodPerformenceUp, "E_periodPerformenceUp", j, false);
        JsonSerialize(periodTimingUp, "F_periodTimingUp", j, false);
        JsonSerialize(periodDeviceVersion, "G_periodDeviceVersion", j, false);
        JsonSerialize(periodAlarmMonitor, "H_periodAlarmMonitor", j, false);
        JsonSerialize(periodOmWorkParamConfigerMonitor, "I_periodOmWorkParamConfigerMonitor", j, false);
        JsonSerialize(periodAlarmPublishInterval, "J_periodAlarmPublishInterval", j, false);
        JsonSerialize(periodPublishCameraInterExterParamInterval, "K_periodPublishCameraInterExterParamInterval", j, false);
        JsonSerialize(periodGetCameraInterExterParamInterval, "L_periodGetCameraInterExterParamInterval", j, false);
        JsonSerialize(periodQueryCameraInterExterParamInterval, "M_periodQueryCameraInterExterParamInterval", j, false);

        JsonSerialize(periodPublishRadarInterExterParamInterval, "N_periodPublishRadarInterExterParamInterval", j, false);
        JsonSerialize(periodQueryRadarInterExterParamInterval, "O_periodQueryRadarInterExterParamInterval", j, false);

        JsonSerialize(periodPublishTrafficlightDetectDataInterval, "P_periodPublishTrafficlightDetectDataInterval", j, false);
        JsonSerialize(publishAlarmDataIntervalUnit, "Q_publishAlarmDataIntervalUnit", j, false);

        JsonSerialize(periodMonitorCameraHeartBeatUp, "R_periodMonitorCameraHeartBeatUp", j, false);
        JsonSerialize(periodMonitorRadarHeartBeatUp, "S_periodMonitorRadarHeartBeatUp", j, false);
        JsonSerialize(periodMonitorPerformanceData, "T_periodMonitorPerformanceData", j, false);

        JsonSerialize(periodMonitorCameraHeartBeatForAlarm, "U_periodMonitorCameraHeartBeatForAlarm", j, false);
        JsonSerialize(periodMonitorRadarHeartBeatForAlarm, "U_periodMonitorRadarHeartBeatForAlarm", j, false);
        JsonSerialize(periodMonitorOmStatus, "V_periodMonitorOmStatus", j, false);
        JsonSerialize(periodPingSensorDeviceActive, "W_periodPingSensorDeviceActive", j, false);

        JsonSerialize(periodCheckMecStatus, "X_periodCheckMecStatus", j, false);
        JsonSerialize(periodCheckSpatDataStatus, "Y_periodCheckSpatDataStatus", j, false);
        JsonSerialize(periodSpatDataStatusUploadInterval, "Z_periodSpatDataStatusUploadInterval", j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(periodHeartBeatUp, "A_PeriodHeartBeatUp", j, noUse_isEmptyFlag);
        JsonDeserialize(periodRunningInfoUp, "B_PeriodRunningInfoUp", j, noUse_isEmptyFlag);
        JsonDeserialize(periodAlarmInfoUp, "C_PeriodAlarmInfoUp", j, noUse_isEmptyFlag);
        JsonDeserialize(periodDownloadBin, "D_PeriodDownloadBin", j, noUse_isEmptyFlag);
        JsonDeserialize(periodPerformenceUp, "E_periodPerformenceUp", j, noUse_isEmptyFlag);
        JsonDeserialize(periodTimingUp, "F_periodTimingUp", j, noUse_isEmptyFlag);
        JsonDeserialize(periodDeviceVersion, "G_periodDeviceVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(periodAlarmMonitor, "H_periodAlarmMonitor", j, noUse_isEmptyFlag);
        JsonDeserialize(periodOmWorkParamConfigerMonitor, "I_periodOmWorkParamConfigerMonitor", j, noUse_isEmptyFlag);
        JsonDeserialize(periodAlarmPublishInterval, "J_periodAlarmPublishInterval", j, noUse_isEmptyFlag);
        JsonDeserialize(periodPublishCameraInterExterParamInterval, "K_periodPublishCameraInterExterParamInterval", j, noUse_isEmptyFlag);
        JsonDeserialize(periodGetCameraInterExterParamInterval, "L_periodGetCameraInterExterParamInterval", j, noUse_isEmptyFlag);
        JsonDeserialize(periodQueryCameraInterExterParamInterval, "M_periodQueryCameraInterExterParamInterval", j, noUse_isEmptyFlag);


        JsonDeserialize(periodPublishRadarInterExterParamInterval, "N_periodPublishRadarInterExterParamInterval", j, noUse_isEmptyFlag);
        JsonDeserialize(periodQueryRadarInterExterParamInterval, "O_periodQueryRadarInterExterParamInterval", j, noUse_isEmptyFlag);
        JsonDeserialize(periodPublishTrafficlightDetectDataInterval, "P_periodPublishTrafficlightDetectDataInterval", j, noUse_isEmptyFlag);
        JsonDeserialize(publishAlarmDataIntervalUnit, "Q_publishAlarmDataIntervalUnit", j, noUse_isEmptyFlag);

        JsonDeserialize(periodMonitorCameraHeartBeatUp, "R_periodMonitorCameraHeartBeatUp", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorRadarHeartBeatUp, "S_periodMonitorRadarHeartBeatUp", j, noUse_isEmptyFlag);

        JsonDeserialize(periodMonitorPerformanceData, "T_periodMonitorPerformanceData", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorCameraHeartBeatForAlarm, "U_periodMonitorCameraHeartBeatForAlarm", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorRadarHeartBeatForAlarm, "U_periodMonitorRadarHeartBeatForAlarm", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorOmStatus, "V_periodMonitorOmStatus", j, noUse_isEmptyFlag);

        JsonDeserialize(periodPingSensorDeviceActive, "W_periodPingSensorDeviceActive", j, noUse_isEmptyFlag);

        JsonDeserialize(periodCheckMecStatus, "X_periodCheckMecStatus", j, noUse_isEmptyFlag);
        JsonDeserialize(periodCheckSpatDataStatus, "Y_periodCheckSpatDataStatus", j, noUse_isEmptyFlag);

        JsonDeserialize(periodSpatDataStatusUploadInterval, "Z_periodSpatDataStatusUploadInterval", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_periodHeartBeatUp: " << periodHeartBeatUp <<  std::endl;
        ss << std::left << std::setw(40) << "B_periodRunningInfoUp: " << periodRunningInfoUp <<  std::endl;
        ss << std::left << std::setw(40) << "C_periodAlarmInfoUp: " << periodAlarmInfoUp <<  std::endl;
        ss << std::left << std::setw(40) << "D_periodDownloadBin: " << periodDownloadBin <<  std::endl;
        ss << std::left << std::setw(40) << "E_periodPerformenceUp: " << periodPerformenceUp <<  std::endl;
        ss << std::left << std::setw(40) << "F_periodTimingUp: " << periodTimingUp <<  std::endl;
        ss << std::left << std::setw(40) << "G_periodDeviceVersion: " << periodDeviceVersion <<  std::endl;
        ss << std::left << std::setw(40) << "H_periodAlarmMonitor: " << periodAlarmMonitor <<  std::endl;
        ss << std::left << std::setw(40) << "I_periodOmWorkParamConfigerMonitor: " << periodOmWorkParamConfigerMonitor <<  std::endl;
        ss << std::left << std::setw(40) << "J_periodAlarmPublishInterval: " << periodAlarmPublishInterval <<  std::endl;
        ss << std::left << std::setw(40) << "K_periodPublishCameraInterExterParamInterval: " << periodPublishCameraInterExterParamInterval <<  std::endl;
        ss << std::left << std::setw(40) << "L_periodGetCameraInterExterParamInterval: " << periodGetCameraInterExterParamInterval <<  std::endl;
        ss << std::left << std::setw(40) << "M_periodQueryCameraInterExterParamInterval: " << periodQueryCameraInterExterParamInterval <<  std::endl;

        ss << std::left << std::setw(40) << "N_periodPublishRadarInterExterParamInterval: " << periodPublishRadarInterExterParamInterval <<  std::endl;
        ss << std::left << std::setw(40) << "O_periodQueryRadarInterExterParamInterval: " << periodQueryRadarInterExterParamInterval <<  std::endl;
        ss << std::left << std::setw(40) << "P_periodPublishTrafficlightDetectDataInterval: " << periodPublishTrafficlightDetectDataInterval <<  std::endl;
        ss << std::left << std::setw(40) << "Q_publishAlarmDataIntervalUnit: " << publishAlarmDataIntervalUnit <<  std::endl;

        ss << std::left << std::setw(40) << "R_periodMonitorCameraHeartBeatUp: " << periodMonitorCameraHeartBeatUp <<  std::endl;
        ss << std::left << std::setw(40) << "S_periodMonitorRadarHeartBeatUp: " << periodMonitorRadarHeartBeatUp <<  std::endl;

        ss << std::left << std::setw(40) << "T_periodMonitorPerformanceData: " << periodMonitorPerformanceData <<  std::endl;
        ss << std::left << std::setw(40) << "U_periodMonitorCameraHeartBeatForAlarm: " << periodMonitorCameraHeartBeatForAlarm <<  std::endl;
        ss << std::left << std::setw(40) << "U_periodMonitorRadarHeartBeatForAlarm: " << periodMonitorRadarHeartBeatForAlarm <<  std::endl;
        ss << std::left << std::setw(40) << "V_periodMonitorOmStatus: " << periodMonitorOmStatus <<  std::endl;
        ss << std::left << std::setw(40) << "W_periodPingSensorDeviceActive: " << periodPingSensorDeviceActive <<  std::endl;
        ss << std::left << std::setw(40) << "X_periodCheckMecStatus: " << periodCheckMecStatus <<  std::endl;

        ss << std::left << std::setw(40) << "Y_periodCheckSpatDataStatus: " << periodCheckSpatDataStatus <<  std::endl;
        ss << std::left << std::setw(40) << "Y_periodSpatDataStatusUploadInterval: " << periodSpatDataStatusUploadInterval <<  std::endl;
        return ss.str();
    }
};
struct OmConfigerCommon : public afl::base::SerializableData
{
public:
    std::string protocolVersion = "v1.0";
    std::string softwareVersion = "airos-v1.5-202406010601";
    std::string hardwareVersion = "v3.0";
    std::string regionId = "110113";
    double      longitude = 116.3212341;
    double      latitude = 33.2614561;
    double      elevation = 66.2;

    int         roadId = NUM_DEFAULT_VALUE;
    std::string roadName  = STR_DEFAULT_VALUE;
    int         roadType = NUM_DEFAULT_VALUE;
    std::string crossId  = STR_DEFAULT_VALUE;
    int         crossType = NUM_DEFAULT_VALUE;
    std::string crossName  = STR_DEFAULT_VALUE;
    int         linkId = NUM_DEFAULT_VALUE;
    std::string linkName  = STR_DEFAULT_VALUE;
    int         deviceType = NUM_DEFAULT_VALUE;
    std::string supplier  = STR_DEFAULT_VALUE;
    std::string owner  = STR_DEFAULT_VALUE;
    std::string iccId  = STR_DEFAULT_VALUE;
    std::string imei = STR_DEFAULT_VALUE;
    int         transProtocal = 5;
    std::string pointNo  = "101";
    std::string pointName = "102";
    std::string deviceName = "TC001";
    double      height = 1.5;
    int         siteType = 0;
    std::string addressIP = "";
    std::string netMask = "";
    std::string gateway = "";
    int         logLevel = -1;
    int         active = 0;
    int         sensorNum = 0;

private:
    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(protocolVersion, "A_ProtocolVersion", j, false);
        JsonSerialize(softwareVersion, "B_SoftwareVersion", j, false);
        JsonSerialize(hardwareVersion, "C_HardwareVersion", j, false);
        JsonSerialize(regionId, "D_RegionId", j, false);
        JsonSerialize(longitude, "E_Longitude", j, false);
        JsonSerialize(latitude, "F_Latitude", j, false);
        JsonSerialize(elevation, "G_Elevation", j, false);
        JsonSerialize(roadId, "H_RoadId", j, false);
        JsonSerialize(roadName, "I_RoadName", j, false);
        JsonSerialize(roadType, "J_RoadType", j, false);
        JsonSerialize(crossId, "K_CrossId", j, false);
        JsonSerialize(crossType, "L_CrossType", j, false);
        JsonSerialize(crossName, "M_CrossName", j, false);
        JsonSerialize(linkId, "N_LinkId", j, false);
        JsonSerialize(linkName, "O_LinkName", j, false);
        JsonSerialize(deviceType, "P_DeviceType", j, false);
        JsonSerialize(supplier, "Q_Supplier", j, false);
        JsonSerialize(owner, "R_Owner", j, false);
        JsonSerialize(iccId, "S_IccId", j, false);
        JsonSerialize(imei, "T_Imei", j, false);
        JsonSerialize(transProtocal, "U_TransProtocal", j, false);
        JsonSerialize(pointNo, "V_PointNo", j, false);
        JsonSerialize(pointName, "W_PointName", j, false);
        JsonSerialize(deviceName, "X_DeviceName", j, false);
        JsonSerialize(height, "Y_Height", j, false);
        JsonSerialize(siteType, "Z_SiteType", j, false);
        JsonSerialize(addressIP, "AA_AddressIP", j, false);
        JsonSerialize(netMask, "AB_NetMask", j, false);
        JsonSerialize(gateway, "AC_Gateway", j, false);
        JsonSerialize(logLevel, "AD_LogLevel", j, false);
        
        JsonSerialize(active, "AE_active", j, false);
        JsonSerialize(sensorNum, "AF_sensorNum", j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(protocolVersion, "A_ProtocolVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(softwareVersion, "B_SoftwareVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(hardwareVersion, "C_HardwareVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(regionId, "D_RegionId", j, noUse_isEmptyFlag);
        JsonDeserialize(longitude, "E_Longitude", j, noUse_isEmptyFlag);
        JsonDeserialize(latitude, "F_Latitude", j, noUse_isEmptyFlag);
        JsonDeserialize(elevation, "G_Elevation", j, noUse_isEmptyFlag);
        JsonDeserialize(roadId, "H_RoadId", j, noUse_isEmptyFlag);
        JsonDeserialize(roadName, "I_RoadName", j, noUse_isEmptyFlag);
        JsonDeserialize(roadType, "J_RoadType", j, noUse_isEmptyFlag);
        JsonDeserialize(crossId, "K_CrossId", j, noUse_isEmptyFlag);
        JsonDeserialize(crossType, "L_CrossType", j, noUse_isEmptyFlag);
        JsonDeserialize(crossName, "M_CrossName", j, noUse_isEmptyFlag);
        JsonDeserialize(linkId, "N_LinkId", j, noUse_isEmptyFlag);
        JsonDeserialize(linkName, "O_LinkName", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceType, "P_DeviceType", j, noUse_isEmptyFlag);
        JsonDeserialize(supplier, "Q_Supplier", j, noUse_isEmptyFlag);
        JsonDeserialize(owner, "R_Owner", j, noUse_isEmptyFlag);
        JsonDeserialize(iccId, "S_IccId", j, noUse_isEmptyFlag);
        JsonDeserialize(imei, "T_Imei", j, noUse_isEmptyFlag);
        JsonDeserialize(transProtocal, "U_TransProtocal", j, noUse_isEmptyFlag);
        JsonDeserialize(pointNo, "V_PointNo", j, noUse_isEmptyFlag);
        JsonDeserialize(pointName, "W_PointName", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceName, "X_DeviceName", j, noUse_isEmptyFlag);
        JsonDeserialize(height, "Y_Height", j, noUse_isEmptyFlag);
        JsonDeserialize(siteType, "Z_SiteType", j, noUse_isEmptyFlag);
        JsonDeserialize(addressIP, "AA_AddressIP", j, noUse_isEmptyFlag);
        JsonDeserialize(netMask, "AB_NetMask", j, noUse_isEmptyFlag);
        JsonDeserialize(gateway, "AC_Gateway", j, noUse_isEmptyFlag);
        JsonDeserialize(logLevel, "AD_LogLevel", j, noUse_isEmptyFlag);
        
        JsonDeserialize(active, "AE_active", j, noUse_isEmptyFlag);
        JsonDeserialize(sensorNum, "AF_sensorNum", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss <<  std::endl << std::left << std::setw(40) << "ProtocolVersion: " << protocolVersion <<  std::endl;
        ss << std::left << std::setw(40) << "SoftwareVersion: " << softwareVersion <<  std::endl;
        ss << std::left << std::setw(40) << "HardwareVersion: " << hardwareVersion <<  std::endl;
        ss << std::left << std::setw(40) << "RegionId: " << regionId <<  std::endl;
        ss << std::left << std::setw(40) << "Longitude: " << longitude <<  std::endl;
        ss << std::left << std::setw(40) << "Latitude: " << latitude <<  std::endl;
        ss << std::left << std::setw(40) << "Elevation: " << elevation <<  std::endl;
        ss << std::left << std::setw(40) << "RoadId: " << roadId <<  std::endl;
        ss << std::left << std::setw(40) << "RoadName: " << roadName <<  std::endl;
        ss << std::left << std::setw(40) << "RoadType: " << roadType <<  std::endl;
        ss << std::left << std::setw(40) << "CrossId: " << crossId <<  std::endl;
        ss << std::left << std::setw(40) << "CrossType: " << crossType <<  std::endl;
        ss << std::left << std::setw(40) << "CrossName: " << crossName <<  std::endl;
        ss << std::left << std::setw(40) << "LinkId: " << linkId <<  std::endl;
        ss << std::left << std::setw(40) << "LinkName: " << linkName <<  std::endl;
        ss << std::left << std::setw(40) << "DeviceType: " << deviceType <<  std::endl;
        ss << std::left << std::setw(40) << "Supplier: " << supplier <<  std::endl;
        ss << std::left << std::setw(40) << "Owner: " << owner <<  std::endl;
        ss << std::left << std::setw(40) << "IccId: " << iccId <<  std::endl;
        ss << std::left << std::setw(40) << "Imei: " << imei <<  std::endl;
        ss << std::left << std::setw(40) << "TransProtocal: " << transProtocal <<  std::endl;
        ss << std::left << std::setw(40) << "PointNo: " << pointNo <<  std::endl;
        ss << std::left << std::setw(40) << "PointName: " << pointName <<  std::endl;
        ss << std::left << std::setw(40) << "DeviceName: " << deviceName <<  std::endl;
        ss << std::left << std::setw(40) << "Height: " << height <<  std::endl;
        ss << std::left << std::setw(40) << "SiteType: " << siteType <<  std::endl;
        ss << std::left << std::setw(40) << "AddressIP: " << addressIP <<  std::endl;
        ss << std::left << std::setw(40) << "NetMask: " << netMask <<  std::endl;
        ss << std::left << std::setw(40) << "Gateway: " << gateway <<  std::endl;
        ss << std::left << std::setw(40) << "LogLevel: " << logLevel <<  std::endl;

        
        ss << std::left << std::setw(40) << "Active: " << active <<  std::endl;
        ss << std::left << std::setw(40) << "SensorNum: " << sensorNum <<  std::endl;
        return ss.str();
    }
};
struct MecBsHttpServerConfiger: public afl::base::SerializableData
{
public:
    string videoAndPicturePath = "/ftp/video";
    string ftpUrl = "ftp://ftpuser:123456@192.168.6.128:21/video";
    string postPath = "/test";
    string httpHost = "172.22.67.111";
    int httpPort = 8080;
    std::string videoAndPictureAllPath;
    std::string ftpUrlAllPath;
    std::string httpPostPath;
private:
    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(videoAndPicturePath, "B_videoAndPicturePath", j, false);
        JsonSerialize(ftpUrl, "C_ftpUrl", j, false);
        JsonSerialize(postPath, "D_postPath", j, false);
        JsonSerialize(httpHost, "E_httpHost", j, false);
        JsonSerialize(httpPort, "F_httpPort", j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(videoAndPicturePath, "B_videoAndPicturePath", j, noUse_isEmptyFlag);
        JsonDeserialize(ftpUrl, "C_ftpUrl", j, noUse_isEmptyFlag);
        JsonDeserialize(postPath, "D_postPath", j, noUse_isEmptyFlag);
        JsonDeserialize(httpHost, "E_httpHost", j, noUse_isEmptyFlag);
        JsonDeserialize(httpPort, "F_httpPort", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss  << std::left << std::setw(40) << "VideoAndPicturePath: " << videoAndPicturePath <<  std::endl;
        ss << std::left << std::setw(40) << "FtpUrl: " << ftpUrl <<  std::endl;
        ss << std::left << std::setw(40) << "PostPath: " << postPath <<  std::endl;
        ss << std::left << std::setw(40) << "HttpHost: " << httpHost <<  std::endl;
        ss << std::left << std::setw(40) << "HttpPort: " << httpPort <<  std::endl;
        return ss.str();
    }
};

//ota升级参数配置
struct OtaParamConfiger : public afl::base::SerializableData
{
    bool                airosVersionTarFlag = false;
    std::string         airosVersionSeqNum = "";
    std::string         airosVersionFromFile = "";
    // std::string         airosVersionFromOtaDownData = ""; //ota版本：来自ota升级命令中
    // std::string         airosVersionUsing = ""; //ota版本：正在使用中，第一次等于airosVersionFromFile，
    // //后期等于升级成功后的airosVersionFromOtaDownData
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(airosVersionTarFlag, "A_airosVersionTarFlag", j, false);
        JsonSerialize(airosVersionSeqNum, "B_airosVersionSeqNum", j, false);
        JsonSerialize(airosVersionFromFile, "C_airosVersionFromFile", j, false);

        // JsonSerialize(airosVersionFromOtaDownData, "D_airosVersionFromOtaDownData", j, false);
        // JsonSerialize(airosVersionUsing, "E_airosVersionUsing", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(airosVersionTarFlag, "A_airosVersionTarFlag", j, noUse_isEmptyFlag);
        JsonDeserialize(airosVersionSeqNum, "B_airosVersionSeqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(airosVersionFromFile, "C_airosVersionFromFile", j, noUse_isEmptyFlag);

        // JsonDeserialize(airosVersionFromOtaDownData, "D_airosVersionFromOtaDownData", j, noUse_isEmptyFlag);
        // JsonDeserialize(airosVersionUsing, "E_airosVersionUsing", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "airosVersionTarFlag: " << airosVersionTarFlag << std::endl;
        ss << std::left << std::setw(40) << "airosVersionSeqNum: " << airosVersionSeqNum << std::endl;
        ss << std::left << std::setw(40) << "airosVersionFromFile: " << airosVersionFromFile << std::endl;
        return ss.str();
    }
};

//ota、设备重启中时间保存
struct OtaAndRebootTimeConfiger : public afl::base::SerializableData
{
public:
    int     OtaUpdateUtcMillSecondTime = 0;
    bool    OtaNeedExec = false;
    int     DeviceRebootUtcMillSecondTime = 0;
    bool    DeviceRebootNeedExec = false;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(OtaUpdateUtcMillSecondTime, "A_OtaUpdateTime", j, false);
        JsonSerialize(OtaNeedExec, "B_OtaNeedExec", j, false);
        JsonSerialize(DeviceRebootUtcMillSecondTime, "C_DeviceRebootTime", j, false);
        JsonSerialize(DeviceRebootNeedExec, "D_DeviceRebootNeedExec", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(OtaUpdateUtcMillSecondTime, "A_OtaUpdateTime", j, noUse_isEmptyFlag);
        JsonDeserialize(OtaNeedExec, "B_OtaNeedExec", j, noUse_isEmptyFlag);
        JsonDeserialize(DeviceRebootUtcMillSecondTime, "C_DeviceRebootTime", j, noUse_isEmptyFlag);
        JsonDeserialize(DeviceRebootNeedExec, "D_DeviceRebootNeedExec", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "OtaUpdateTime: " << OtaUpdateUtcMillSecondTime <<  std::endl;
        ss << std::left << std::setw(40) << "OtaNeedExec: " << OtaNeedExec <<  std::endl;
        ss << std::left << std::setw(40) << "DeviceRebootTime: " << DeviceRebootUtcMillSecondTime <<  std::endl;
        ss << std::left << std::setw(40) << "DeviceRebootNeedExec: " << DeviceRebootNeedExec <<  std::endl;
        return ss.str();
    }
};

struct TimeRule  : public afl::base::SerializableData
{
    uint64_t timeoutSecond;                 // 超时触发阈值 (秒)
    uint64_t recoverySecond;                // 恢复所需持续时间 (秒)
    bool     enable;                        //是否检测
    uint64_t maxTimeoutTolerance;          //
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(timeoutSecond,                      "timeoutSecond",                      j, false);
        JsonSerialize(recoverySecond,                         "recoverySecond",                         j, false);
        JsonSerialize(enable,                         "enable",                         j, false);
        JsonSerialize(maxTimeoutTolerance,                         "maxTimeoutTolerance",                         j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(timeoutSecond,                    "timeoutSecond",                      j, noUse_isEmptyFlag);
        JsonDeserialize(recoverySecond,                       "recoverySecond",                         j, noUse_isEmptyFlag);
        JsonDeserialize(enable,                         "enable",                         j, noUse_isEmptyFlag);
        JsonDeserialize(maxTimeoutTolerance,                         "maxTimeoutTolerance",                         j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timeoutSecond: "           << timeoutSecond           << std::endl;
        ss << std::left << std::setw(40) << "recoverySecond: "              << recoverySecond              << std::endl;
        ss << std::left << std::setw(40) << "enable: "              << (enable?"true":"false")              << std::endl;
        ss << std::left << std::setw(40) << "maxTimeoutTolerance: "              << maxTimeoutTolerance   << std::endl;
        return ss.str();
    }
};
struct MecSelfCheckTime : public afl::base::SerializableData
{
    //信号机原始数据
    TimeRule  spat_src_data_tr;
    //感知目标
    TimeRule  sensor_objs_data_tr;
    //v2x信控数据: 需要监控融合（信控和雷达）之后的信控数据
    TimeRule  ccindex_tc_data_tr;
    //雷达动态数据
    TimeRule  ccindex_tm_data_tr;
    TimeRule  ccindex_tm_data_trajectories_tr;  //实时轨迹数据
    TimeRule  ccindex_tm_data_vehiclePass_tr;   //实时过车数据
    TimeRule  ccindex_tm_data_queueUp_tr;       //实时排队数据
    TimeRule  ccindex_tm_data_areaState_tr;     //实时区域状态数据
    TimeRule  ccindex_tm_data_overflow_tr;      //实时溢出数据
    TimeRule  ccindex_tm_data_outlane_tr;       //实时出口通道数据
    TimeRule  ccindex_tm_data_statistics_tr;    //定时统计数据
    TimeRule  ccindex_tm_data_evaluations_tr;   //定时评价数据
    TimeRule  ccindex_tm_data_nonmotor_tr;      //定时行人及非机动车数据
    TimeRule  ccindex_tm_data_deviceStatus_tr;  //实时设备状态

    TimeRule  ccindex_st_data_tr;
    TimeRule  cloud_point_data_tr;
    TimeRule  sensor_cloud_link_tr;
    TimeRule  ccindex_st_link_tr;
    TimeRule  ccindex_tm_link_tr;
    TimeRule  cloud_point_link_tr;
    MecSelfCheckTime()
    {
























































































		spat_src_data_tr.timeoutSecond = 5;
        spat_src_data_tr.recoverySecond = 5;
        spat_src_data_tr.enable = true;
        spat_src_data_tr.maxTimeoutTolerance = 3;

        sensor_objs_data_tr.timeoutSecond = 10;
        sensor_objs_data_tr.recoverySecond = 10;
        sensor_objs_data_tr.enable = true;
        sensor_objs_data_tr.maxTimeoutTolerance = 3;


        ccindex_tc_data_tr.timeoutSecond = 15;
        ccindex_tc_data_tr.recoverySecond = 15;
        ccindex_tc_data_tr.enable = true;
        ccindex_tc_data_tr.maxTimeoutTolerance = 3;

        ccindex_tm_data_tr.timeoutSecond = 3;
        ccindex_tm_data_tr.recoverySecond = 3;
        ccindex_tm_data_tr.enable = true;
        ccindex_tm_data_tr.maxTimeoutTolerance = 3;
        //实时轨迹数据
        ccindex_tm_data_trajectories_tr.timeoutSecond = 3;
        ccindex_tm_data_trajectories_tr.recoverySecond = 3;
        ccindex_tm_data_trajectories_tr.enable = true;
        sensor_objs_data_tr.maxTimeoutTolerance = 3;
        //实时过车数据
        ccindex_tm_data_vehiclePass_tr.timeoutSecond = 3;
        ccindex_tm_data_vehiclePass_tr.recoverySecond = 3;
        ccindex_tm_data_vehiclePass_tr.enable = true;
        ccindex_tm_data_vehiclePass_tr.maxTimeoutTolerance = 3;

        //实时排队数据
        ccindex_tm_data_queueUp_tr.timeoutSecond = 3;
        ccindex_tm_data_queueUp_tr.recoverySecond = 3;
        ccindex_tm_data_queueUp_tr.enable = true;
        ccindex_tm_data_queueUp_tr.maxTimeoutTolerance = 3;
        //实时区域状态数据
        ccindex_tm_data_areaState_tr.timeoutSecond = 3;
        ccindex_tm_data_areaState_tr.recoverySecond = 3;
        ccindex_tm_data_areaState_tr.enable = true;
        ccindex_tm_data_areaState_tr.maxTimeoutTolerance = 3;
        //实时溢出数据
        ccindex_tm_data_overflow_tr.timeoutSecond = 3;
        ccindex_tm_data_overflow_tr.recoverySecond = 3;
        ccindex_tm_data_overflow_tr.enable = true;
        ccindex_tm_data_overflow_tr.maxTimeoutTolerance = 3;
        //实时出口通道数据
        ccindex_tm_data_outlane_tr.timeoutSecond = 3;
        ccindex_tm_data_outlane_tr.recoverySecond = 3;
        ccindex_tm_data_outlane_tr.enable = true;
        ccindex_tm_data_outlane_tr.maxTimeoutTolerance = 3;

        //定时统计数据
        ccindex_tm_data_statistics_tr.timeoutSecond = 320;
        ccindex_tm_data_statistics_tr.recoverySecond = 320;
        ccindex_tm_data_statistics_tr.enable = true;
        ccindex_tm_data_statistics_tr.maxTimeoutTolerance = 3;
        //定时评价数据
        ccindex_tm_data_evaluations_tr.timeoutSecond = 65;
        ccindex_tm_data_evaluations_tr.recoverySecond = 65;
        ccindex_tm_data_evaluations_tr.enable = true;
        ccindex_tm_data_evaluations_tr.maxTimeoutTolerance = 3;
        //定时行人及非机动车数据
        ccindex_tm_data_nonmotor_tr.timeoutSecond = 45;
        ccindex_tm_data_nonmotor_tr.recoverySecond = 45;
        ccindex_tm_data_nonmotor_tr.enable = true;
        ccindex_tm_data_nonmotor_tr.maxTimeoutTolerance = 3;
        //实时设备状态
        ccindex_tm_data_deviceStatus_tr.timeoutSecond = 360;
        ccindex_tm_data_deviceStatus_tr.recoverySecond = 360;
        ccindex_tm_data_deviceStatus_tr.enable = true;
        ccindex_tm_data_deviceStatus_tr.maxTimeoutTolerance = 3;


        ccindex_st_data_tr.timeoutSecond = 920;
        ccindex_st_data_tr.recoverySecond = 920;
        ccindex_st_data_tr.enable = true;
        ccindex_st_data_tr.maxTimeoutTolerance = 3;

        cloud_point_data_tr.timeoutSecond = 3610;
        cloud_point_data_tr.recoverySecond = 3610;
        cloud_point_data_tr.enable = true;
        cloud_point_data_tr.maxTimeoutTolerance = 3;

        sensor_cloud_link_tr.timeoutSecond = 5;
        sensor_cloud_link_tr.recoverySecond = 5;
        sensor_cloud_link_tr.enable = true;
        sensor_cloud_link_tr.maxTimeoutTolerance = 3;

        ccindex_st_link_tr.timeoutSecond = 310;
        ccindex_st_link_tr.recoverySecond = 310;
        ccindex_st_link_tr.enable = true;
        ccindex_st_link_tr.maxTimeoutTolerance = 3;

        ccindex_tm_link_tr.timeoutSecond = 5;
        ccindex_tm_link_tr.recoverySecond = 5;
        ccindex_tm_link_tr.enable = true;
        ccindex_tm_link_tr.maxTimeoutTolerance = 3;

        cloud_point_link_tr.timeoutSecond = 3610;
        cloud_point_link_tr.recoverySecond = 3610;
        cloud_point_link_tr.enable = true;
        cloud_point_link_tr.maxTimeoutTolerance = 3;

    }

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(spat_src_data_tr,                      "A_spat_src_data_tr",                      j, false);
        JsonSerialize(sensor_objs_data_tr,                   "B_sensor_objs_data_tr",                        j, false);
        JsonSerialize(ccindex_tc_data_tr,                    "C_ccindex_tc_data_tr",                         j, false);
        JsonSerialize(ccindex_tm_data_tr,                    "D_ccindex_tm_data_tr",                         j, false);
        JsonSerialize(ccindex_st_data_tr,                    "E_ccindex_st_data_tr",                         j, false);
        JsonSerialize(cloud_point_data_tr,                   "F_cloud_point_data_tr",                   j, false);
        JsonSerialize(sensor_cloud_link_tr,                  "G_sensor_cloud_link_tr",                  j, false);
        JsonSerialize(ccindex_st_link_tr,                    "H_ccindex_st_link_tr",                    j, false);
        JsonSerialize(ccindex_tm_link_tr,                    "I_ccindex_tm_link_tr",                    j, false);
        JsonSerialize(cloud_point_link_tr,                   "J_cloud_point_link_tr",                   j, false);

        // --- 以下为补充的缺失字段 ---
        JsonSerialize(ccindex_tm_data_trajectories_tr,       "K_ccindex_tm_data_trajectories_tr",       j, false);
        JsonSerialize(ccindex_tm_data_vehiclePass_tr,        "L_ccindex_tm_data_vehiclePass_tr",        j, false);
        JsonSerialize(ccindex_tm_data_queueUp_tr,            "M_ccindex_tm_data_queueUp_tr",            j, false);
        JsonSerialize(ccindex_tm_data_areaState_tr,          "N_ccindex_tm_data_areaState_tr",          j, false);
        JsonSerialize(ccindex_tm_data_overflow_tr,           "O_ccindex_tm_data_overflow_tr",           j, false);
        JsonSerialize(ccindex_tm_data_outlane_tr,            "P_ccindex_tm_data_outlane_tr",            j, false);
        JsonSerialize(ccindex_tm_data_statistics_tr,         "Q_ccindex_tm_data_statistics_tr",         j, false);
        JsonSerialize(ccindex_tm_data_evaluations_tr,        "R_ccindex_tm_data_evaluations_tr",        j, false);
        JsonSerialize(ccindex_tm_data_nonmotor_tr,           "S_ccindex_tm_data_nonmotor_tr",           j, false);
        JsonSerialize(ccindex_tm_data_deviceStatus_tr,       "T_ccindex_tm_data_deviceStatus_tr",       j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(spat_src_data_tr,                    "A_spat_src_data_tr",                      j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_objs_data_tr,                 "B_sensor_objs_data_tr",                        j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tc_data_tr,                  "C_ccindex_tc_data_tr",                         j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_data_tr,                  "D_ccindex_tm_data_tr",                         j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_st_data_tr,                  "E_ccindex_st_data_tr",                         j, noUse_isEmptyFlag);
        JsonDeserialize(cloud_point_data_tr,                 "F_cloud_point_data_tr",                   j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_cloud_link_tr,                "G_sensor_cloud_link_tr",                  j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_st_link_tr,                  "H_ccindex_st_link_tr",                    j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_link_tr,                  "I_ccindex_tm_link_tr",                    j, noUse_isEmptyFlag);
        JsonDeserialize(cloud_point_link_tr,                 "J_cloud_point_link_tr",                   j, noUse_isEmptyFlag);

        // --- 以下为补充的缺失字段 ---
        JsonDeserialize(ccindex_tm_data_trajectories_tr,     "K_ccindex_tm_data_trajectories_tr",       j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_data_vehiclePass_tr,      "L_ccindex_tm_data_vehiclePass_tr",        j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_data_queueUp_tr,          "M_ccindex_tm_data_queueUp_tr",            j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_data_areaState_tr,        "N_ccindex_tm_data_areaState_tr",          j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_data_overflow_tr,         "O_ccindex_tm_data_overflow_tr",           j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_data_outlane_tr,          "P_ccindex_tm_data_outlane_tr",            j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_data_statistics_tr,       "Q_ccindex_tm_data_statistics_tr",         j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_data_evaluations_tr,      "R_ccindex_tm_data_evaluations_tr",        j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_data_nonmotor_tr,         "S_ccindex_tm_data_nonmotor_tr",           j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_data_deviceStatus_tr,     "T_ccindex_tm_data_deviceStatus_tr",       j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "spat_src_data_tr: "           << spat_src_data_tr.to_string()           << std::endl;
        ss << std::left << std::setw(40) << "sensor_objs_data_tr: "        << sensor_objs_data_tr.to_string()        << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tc_data_tr: "         << ccindex_tc_data_tr.to_string()         << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tm_data_tr: "         << ccindex_tm_data_tr.to_string()         << std::endl;

        // --- 以下为补充的缺失字段 ---
        ss << std::left << std::setw(40) << "ccindex_tm_data_trajectories_tr: " << ccindex_tm_data_trajectories_tr.to_string() << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tm_data_vehiclePass_tr: "  << ccindex_tm_data_vehiclePass_tr.to_string()  << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tm_data_queueUp_tr: "      << ccindex_tm_data_queueUp_tr.to_string()      << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tm_data_areaState_tr: "    << ccindex_tm_data_areaState_tr.to_string()    << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tm_data_overflow_tr: "     << ccindex_tm_data_overflow_tr.to_string()     << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tm_data_outlane_tr: "      << ccindex_tm_data_outlane_tr.to_string()      << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tm_data_statistics_tr: "   << ccindex_tm_data_statistics_tr.to_string()   << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tm_data_evaluations_tr: "  << ccindex_tm_data_evaluations_tr.to_string()  << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tm_data_nonmotor_tr: "     << ccindex_tm_data_nonmotor_tr.to_string()     << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tm_data_deviceStatus_tr: " << ccindex_tm_data_deviceStatus_tr.to_string() << std::endl;
        // ----------------------------

        ss << std::left << std::setw(40) << "ccindex_st_data_tr: "         << ccindex_st_data_tr.to_string()         << std::endl;
        ss << std::left << std::setw(40) << "cloud_point_data_tr: "        << cloud_point_data_tr.to_string()        << std::endl;
        ss << std::left << std::setw(40) << "sensor_cloud_link_tr: "       << sensor_cloud_link_tr.to_string()       << std::endl;
        ss << std::left << std::setw(40) << "ccindex_st_link_tr: "         << ccindex_st_link_tr.to_string()         << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tm_link_tr: "         << ccindex_tm_link_tr.to_string()         << std::endl;
        ss << std::left << std::setw(40) << "cloud_point_link_tr: "        << cloud_point_link_tr.to_string()        << std::endl;

        return ss.str();
    }
};


struct CameraInterExterParamConfiger : public afl::base::SerializableData
{
public:
    std::string path= "/home/airos/common_config/camera_param";
    std::string fileType = "json";
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(path, "A_path", j, false);
        JsonSerialize(fileType, "B_fileType", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(path, "A_path", j, noUse_isEmptyFlag);
        JsonDeserialize(fileType, "B_fileType", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "path: " << path <<  std::endl;
        ss << std::left << std::setw(40) << "fileType: " << fileType <<  std::endl;
        return ss.str();
    }
};

//rsu客户端配置
struct ConfigerMec : public afl::base::SerializableData
{
    bool                            enableRegister = true;
    bool                            enableUseHearbeatAsOffline = false;
    ScenarioEnableConfigerMec       enableDebugPrintScenario;
    std::string                     nic;
    MqttPublishConfigerPeriod       configerPublishPeriod;
    MqttEnableConfigerMec		    configerEnable;
    MqttTopicConfigerMec	 	    configerTopic;
    ////////////////////////////
    OmConfigerCommon                configerOmCommon;
    MecBsHttpServerConfiger         configerHttpServerMec;
    ////////////////////////////
    DBConfiger                      configerDb;
    AlarmThresholdValue             configAlarmThresholdValue;
    OtaParamConfiger                configerOtaParam;
    AngleOffsetConfiger             configerAngleOffset;
    SpatSrcDataConfiger             configerSpatSrcData;
    V2xDataConfiger                 configerV2xData;
    OtaAndRebootTimeConfiger            configerOtaAndRebootTime;
    TrafficlightDetectDataConfiger     configerTrafficlightDetectData;
    SensorInterExterParamEnablePeriodConfiger configerSensorInterExterParamEnablePeriod;
    MonitorMecConfiger              configerMonitorMec;
    MonitorMecConfigerSpatSrcData       configerMonitorMecSpatSrcData;
    MonitorMecConfigerRsapData          configerMonitorMecRsapData;
    MonitorMecConfigerRsapLink              configerMonitorMecRsapLink;
    MonitorMecConfigerCcindexV2xData  configerMonitorMecCcindexV2xData;
    MonitorMecConfigerCcindexTmData  configerMonitorMecTmData;
    MonitorMecConfigerCcindexTmLink  configerMonitorMecTmLink;
    MonitorMecConfigerCcindexStData  configerMonitorMecStData ;
    MonitorMecConfigerCcindexStLink  configerMonitorMecStLink;
    // MonitorMecConfigerCcindexRcp configerMonitorMecRcp;
    MonitorMecConfigerCcindexRcpData  configerMonitorMecRcpData;
    MonitorMecConfigerCcindexRcpLink  configerMonitorMecRcpLink;
    MecSelfCheckTime                configerMecSelfCheckTime;
    MqttTopicConfigerCCIndex        configerMqttTopicCCIndex;

    MqttTopicConfigerRadarID            configerTopicRadarID;
    CameraInterExterParamConfiger             configerCameraInterExterParam;
    MonitorSpatConfiger                 configerMonitorSpat;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(enableRegister, "A_enableRegister", j, false);
        JsonSerialize(enableUseHearbeatAsOffline, "B_enableUseHearbeatAsOffline", j, false);
        JsonSerialize(enableDebugPrintScenario, "B_enableDebugPrintScenario", j, false);

        JsonSerialize(nic,               "C_nic",                   j, false);
        JsonSerialize(configerPublishPeriod, "D_configerPublishPeriod", j, false);
        JsonSerialize(configerEnable,   "E_configerEnable",   j, false);
        JsonSerialize(configerTopic,    "F_configerTopic",    j, false);
        JsonSerialize(configerOmCommon, "G_configerOmCommon", j, false);
        JsonSerialize(configerHttpServerMec, "H_configerHttpServerMec", j, false);

        JsonSerialize(configerDb, "I_configerDb", j, false);
        JsonSerialize(configAlarmThresholdValue, "J_configAlarmThresholdValue", j, false);
        JsonSerialize(configerOtaParam, "K_configerOtaParam", j, false);
        JsonSerialize(configerAngleOffset, "L_configerAngleOffset", j, false);
        JsonSerialize(configerSpatSrcData, "M_configerSpatSrcData", j, false);
        JsonSerialize(configerV2xData,    "N_configerV2xData", j, false);
        JsonSerialize(configerOtaAndRebootTime, "O_configerOtaAndRebootTime", j, false);
        JsonSerialize(configerTrafficlightDetectData, "P_configerTrafficlightDetectData", j, false);
        JsonSerialize(configerSensorInterExterParamEnablePeriod, "Q_configerSensorInterExterParamEnablePeriod", j, false);
        JsonSerialize(configerMonitorMec, "R_configerMonitorMec", j, false);
        JsonSerialize(configerMonitorMec, "R_configerMonitorMec", j, false);
        JsonSerialize(configerMonitorMecSpatSrcData, "R_configerMonitorMecSpatSrcData", j, false);
        JsonSerialize(configerMonitorMecRsapData, "R_configerMonitorMecRsapData", j, false);
        JsonSerialize(configerMonitorMecRsapLink, "R_configerMonitorMecRsapLink", j, false);
        JsonSerialize(configerMonitorMecCcindexV2xData, "R_configerMonitorMecCcindexV2xData", j, false);
        JsonSerialize(configerMonitorMecTmData, "R_configerMonitorMecTmData", j, false);
        JsonSerialize(configerMonitorMecTmLink, "R_configerMonitorMecTmLink", j, false);
        JsonSerialize(configerMonitorMecStData, "R_configerMonitorMecStData", j, false);
        JsonSerialize(configerMonitorMecStLink, "R_configerMonitorMecStLink", j, false);

        // JsonSerialize(configerMonitorMecRcp, "R_configerMonitorMecRcp", j, false);
        JsonSerialize(configerMonitorMecRcpData, "R_configerMonitorMecRcpData", j, false);
        JsonSerialize(configerMonitorMecRcpLink, "R_configerMonitorMecRcpLink", j, false);

        JsonSerialize(configerMecSelfCheckTime, "S_configerMecSelfCheckTime", j, false);

        JsonSerialize(configerMqttTopicCCIndex, "T_configerMqttTopicCCIndex", j, false);
        JsonSerialize(configerTopicRadarID, "U_configerTopicRadarID", j, false);
        JsonSerialize(configerMonitorSpat, "V_configerMonitorSpat", j, false);
        JsonSerialize(configerCameraInterExterParam, "W_configerCameraInterExterParam", j, false);


    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(enableRegister, "A_enableRegister", j, noUse_isEmptyFlag);
        JsonDeserialize(enableUseHearbeatAsOffline, "B_enableUseHearbeatAsOffline", j, noUse_isEmptyFlag);
        JsonDeserialize(enableDebugPrintScenario, "B_enableDebugPrintScenario", j, noUse_isEmptyFlag);

        JsonDeserialize(nic,                   "C_nic",                   j, noUse_isEmptyFlag);
        JsonDeserialize(configerPublishPeriod, "D_configerPublishPeriod", j, noUse_isEmptyFlag);
        JsonDeserialize(configerEnable,   "E_configerEnable",   j, noUse_isEmptyFlag);
        JsonDeserialize(configerTopic,    "F_configerTopic",    j, noUse_isEmptyFlag);
        JsonDeserialize(configerOmCommon, "G_configerOmCommon", j, noUse_isEmptyFlag);
        JsonDeserialize(configerHttpServerMec, "J_configerHttpServerMec", j, noUse_isEmptyFlag);

        JsonDeserialize(configerDb, "I_configerDb", j, noUse_isEmptyFlag);
        JsonDeserialize(configAlarmThresholdValue, "J_configAlarmThresholdValue", j, noUse_isEmptyFlag);
        JsonDeserialize(configerOtaParam, "K_configerOtaParam", j, noUse_isEmptyFlag);
        JsonDeserialize(configerAngleOffset, "L_configerAngleOffset", j, noUse_isEmptyFlag);
        JsonDeserialize(configerSpatSrcData, "M_configerSpatSrcData", j, noUse_isEmptyFlag);
        JsonDeserialize(configerV2xData,    "N_configerV2xData", j, noUse_isEmptyFlag);
        JsonDeserialize(configerOtaAndRebootTime, "O_configerOtaAndRebootTime", j, noUse_isEmptyFlag);
        JsonDeserialize(configerTrafficlightDetectData, "P_configerTrafficlightDetectData", j, noUse_isEmptyFlag);
        JsonDeserialize(configerSensorInterExterParamEnablePeriod, "Q_configerSensorInterExterParamEnablePeriod", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorMec, "R_configerMonitorMec", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorMecSpatSrcData, "R_configerMonitorMecSpatSrcData", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorMecRsapData, "R_configerMonitorMecRsapData", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorMecRsapLink, "R_configerMonitorMecRsapLink", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorMecCcindexV2xData, "R_configerMonitorMecCcindexV2xData", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorMecTmData, "R_configerMonitorMecTmData", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorMecTmLink, "R_configerMonitorMecTmLink", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorMecStData, "R_configerMonitorMecStData", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorMecStLink, "R_configerMonitorMecStLink", j, noUse_isEmptyFlag);

        // JsonDeserialize(configerMonitorMecRcp, "R_configerMonitorMecRcp", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorMecRcpData, "R_configerMonitorMecRcpData", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorMecRcpLink, "R_configerMonitorMecRcpLink", j, noUse_isEmptyFlag);

        JsonDeserialize(configerMecSelfCheckTime, "S_configerMecSelfCheckTime", j, noUse_isEmptyFlag);

        JsonDeserialize(configerMqttTopicCCIndex, "T_configerMqttTopicCCIndex", j, noUse_isEmptyFlag);

        JsonDeserialize(configerTopicRadarID, "U_configerTopicRadarID", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorSpat, "V_configerMonitorSpat", j, noUse_isEmptyFlag);
        JsonDeserialize(configerCameraInterExterParam, "W_configerCameraInterExterParam", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "enableRegister: " << (enableRegister ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "enableUseHearbeatAsOffline: " << (enableUseHearbeatAsOffline ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "enableDebugPrintScenario: " << enableDebugPrintScenario.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "nic: " << nic <<  std::endl;
        ss << "--------------------------------------------------- " << std::endl;
        ss << std::left << std::setw(40) << "configerPublishPeriod: " << std::endl << std::left  << configerPublishPeriod.to_string() <<  std::endl;
        ss << "--------------------------------------------------- " << std::endl;
        ss << "--------------------------------------------------- " << std::endl;
        ss << std::left << std::setw(40) << "configerEnable: " << std::endl << std::left << configerEnable.to_string() <<  std::endl;
        ss << "--------------------------------------------------- " << std::endl;
        ss << std::left << std::setw(40) << "configerTopic: " << std::endl << std::left << configerTopic.to_string() <<  std::endl;
        ss << "--------------------------------------------------- " << std::endl;
        ss << "--------------------------------------------------- " << std::endl;
        ss << std::left << std::setw(40) << "configerOmCommon: " << std::endl << std::left << configerOmCommon.to_string() <<  std::endl;
        ss << "--------------------------------------------------- " << std::endl;
        ss << std::left << std::setw(40) << "configerHttpServerMec: " << std::endl << std::left  << configerHttpServerMec.to_string() <<  std::endl;
        ss << "--------------------------------------------------- " << std::endl;
        ss << std::left << std::setw(40) << "configerDb: " << std::endl << std::left << configerDb.to_string() <<  std::endl;
        ss << "--------------------------------------------------- " << std::endl;
        ss << std::left << std::setw(40) << "configAlarmThresholdValue: " << std::endl << std::left << configAlarmThresholdValue.to_string() <<  std::endl;
        ss << "--------------------------------------------------- " << std::endl;
        ss << std::left << std::setw(40) << "configerOtaParam: " << std::endl << std::left << configerOtaParam.to_string() <<  std::endl;
        ss << "----------------------------------------" << std::endl;
        ss << std::left << std::setw(40)  << "configerAngleOffset:" << std::endl  << configerAngleOffset.to_string() << std::endl;
        ss << "----------------------------------------" << std::endl;
        ss << std::left << std::setw(40)  << "configerSpatSrcData:" << std::endl  << configerSpatSrcData.to_string() << std::endl;
        ss << "----------------------------------------" << std::endl;
        ss << std::left << std::setw(40)  << "configerV2xData: " << std::endl  << configerV2xData.to_string() << std::endl;
        ss << "----------------------------------------" << std::endl;
        ss << std::left << std::setw(40) << "configerOtaAndRebootTime: " << std::endl << std::left << configerOtaAndRebootTime.to_string() <<  std::endl;
        ss << "----------------------------------------" << std::endl;
        ss << std::left << std::setw(40) << "configerTrafficlightDetectData: " << std::endl << std::left << configerTrafficlightDetectData.to_string() <<  std::endl;
        ss << "----------------------------------------" << std::endl;
        ss << std::left << std::setw(40) << "configerSensorInterExterParamEnablePeriod: " << std::endl << std::left << configerSensorInterExterParamEnablePeriod.to_string() <<  std::endl;
        ss << "----------------------------------------" << std::endl;
        ss << std::left << std::setw(40) << "configerMonitorMec: " << std::endl << std::left << configerMonitorMec.to_string() << std::endl;
        ss << std::left << std::setw(40) << "configerMonitorMecSpatSrcData: " << std::endl << std::left << configerMonitorMecSpatSrcData.to_string() << std::endl;
        ss << std::left << std::setw(40) << "configerMonitorMecRsapData: " << std::endl << std::left << configerMonitorMecRsapData.to_string() << std::endl;
        ss << std::left << std::setw(40) << "configerMonitorMecRsapLink: " << std::endl << std::left << configerMonitorMecRsapLink.to_string() << std::endl;
        ss << std::left << std::setw(40) << "configerMonitorMecCcindexV2xData: " << std::endl << std::left << configerMonitorMecCcindexV2xData.to_string() << std::endl;
        ss << std::left << std::setw(40) << "configerMonitorMecTmData: " << std::endl << std::left << configerMonitorMecTmData.to_string() << std::endl;
        ss << std::left << std::setw(40) << "configerMonitorMecTmLink: " << std::endl << std::left << configerMonitorMecTmLink.to_string() << std::endl;
        ss << std::left << std::setw(40) << "configerMonitorMecStData: " << std::endl << std::left << configerMonitorMecStData.to_string() << std::endl;
        ss << std::left << std::setw(40) << "configerMonitorMecStLink: " << std::endl << std::left << configerMonitorMecStLink.to_string() << std::endl;

        // ss << std::left << std::setw(40) << "configerMonitorMecRcp: " << std::endl << std::left << configerMonitorMecRcp.to_string() << std::endl;
        // ss << std::left << std::setw(40) << "configerMonitorMecRcpData: " << std::endl << std::left << configerMonitorMecRcpData.to_string() << std::endl;
        // ss << std::left << std::setw(40) << "configerMonitorMecRcpLink: " << std::endl << std::left << configerMonitorMecRcpLink.to_string() << std::endl;
        ss << "----------------------------------------" << std::endl;
        ss << std::left << std::setw(40) << "configerMqttTopicCCIndex: " << std::endl << std::left << configerMqttTopicCCIndex.to_string() <<  std::endl;
        ss << "----------------------------------------" << std::endl;
        ss << std::left << std::setw(40) << "configerTopicRadarID: " << std::endl << std::left << configerTopicRadarID.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "configerMonitorSpat: " << std::endl << std::left << configerMonitorSpat.to_string() <<  std::endl;

        ss << std::left << std::setw(40) << "configerCameraInterExterParam:" << std::endl << configerCameraInterExterParam.to_string() << std::endl;
        ss << "----------------------------------------" << std::endl;
        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
