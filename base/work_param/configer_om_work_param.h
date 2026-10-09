/*********************************************************************************
* @file		configer_om_work_param.h
* @brief		configer_om_work_param belongs to CICTCI
* @details
* @author		alfred
* @email       zhangenwei64@gmail.com
* @date		24-6-17
* @copyright	Copyright (c) 2024 CICTCI-Airos Division.
* @verbatim
*
*  Change History:
*  Date      Author    Version  ChangeId           Description
*  ------------------------------------------------------------------------------
*  24-6-17 alfred       1.0       ————             Create this file
*
* @endverbatim
********************************************************************************/

#ifndef AIROS2_0_NEW_CONFIGER_OM_WORK_PARAM_H
#define AIROS2_0_NEW_CONFIGER_OM_WORK_PARAM_H
#include "base/common/network/serializable_data.h"
#include "base/common/network/print.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <memory>
#include "yaml-cpp/yaml.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm> // 需要引入 std::equal
namespace airos {
namespace base {
namespace workparam {
using namespace std;
using namespace afl::base;
using namespace afl::util;
enum WorkParamDeviceType {
    WorkParamDeviceTypeMec = 1,
    WorkParamDeviceTypeCamera = 2,
    WorkParamDeviceTypeRadar = 3
};

struct MecDeviceWorkParam : public afl::base::SerializableData
{
    public:
    int serialNumber = 0; // 序号
    std::string routeId; // 路口编号
    std::string deviceType = "MEC1"; // 设备类型
    std::string deviceSn = "R321432543"; // 设备SN
    std::string deviceEsn = "110220330440550600770"; // 设备ESN
    std::string nic = "eth0"; // 网卡
    std::string mecIp = "172.20.65.184"; // MEC-IP
    int regionID = 0; //路口区域ID
    int crossID = 0; //路口ID
    std::string rsuEuhtIp = "172.20.65.183"; // RSU/EUHT IP
    int rsuEuhtPort = 50501; // RSU/EUHT 端口
    std::string xmlMapFile; // xml地图文件
    std::string rsiConfigFile; // rsi配置下发的文件
    std::string rcuid; // 分配8位设备号

    std::string senseCloudIp; // 感知云控Ip
    int senseCloudPort = 0; // 感知云控端口
    std::string senseCaCertName; // 感知ca证书名
    std::string senseClientCertName; // 感知客户端证书名
    std::string senseClientPrivateKeyFile; // 感知客户端私钥文件名
    std::string senseClientPrivateKeyPwd; // 客户端私钥证书密感知码

    std::string maintenanceCloudUrl = "172.20.65.103"; // 运维云控URL
    std::string maintenanceMqttUsername = "root"; // mqtt用户名
    std::string maintenanceMqttPasswd = "root"; // 运维云控URL
    std::string maintenanceCaCertName = "ca.crt"; // 运维ca证书名
    std::string maintenanceClientCertName = "client.crt"; // 运维客户端证书名
    std::string maintenanceClientPrivateKeyFile = "client.key"; // 运维客户端私钥文件名
    std::string maintenanceClientPrivateKeyPwd = "123456"; // 运维客户端私钥证书密码

    std::string videoFtpUrl = "ftp://ftpuser:123456@192.168.6.128:21/video"; // 视频图片ftpUrl
    std::string videoHttpHost = "172.22.67.111"; //  视频图片httpHost
    int videoHttpPort = 8080; //  视频图片httpPort

    double mecLongitude = 116.1234567; // MEC经度
    double mecLatitude = 32.1234567; // MEC维度
    double mecAltitude = 116.2; // MEC海拔

    std::string radarPointCloudFtpIp; // 雷达点云FTP_IP
    std::string radarPointCloudFtpUser; // 雷达点云FTP_USER
    std::string radarPointCloudFtpPasswd; // 雷达点云FTP_PASSWD

    std::string ccindexCloudUrl = "172.20.65.103"; // 信控URL
    std::string ccindexMqttUsername = "root"; // mqtt用户名
    std::string ccindexMqttPasswd = "root"; // 信控URL
    std::string ccindexCaCertName = "ca.crt"; // 信控ca证书名
    std::string ccindexClientCertName = "client.crt"; // 信控客户端证书名
    std::string ccindexClientPrivateKeyFile = "client.key"; // 信控客户端私钥文件名
    std::string ccindexClientPrivateKeyPwd = "123456"; // 信控客户端私钥证书密码


    std::string lightEsn = "10000";
    std::string euhtEsn = "10000";
    int radarPointCloudFtpPort = 10000;
    std::string license = "";
public:
    virtual void serialize(json &j) override {
        JsonSerialize(serialNumber, "A_serialNumber", j, false);
        JsonSerialize(routeId, "B_routeId", j, false);
        JsonSerialize(deviceType, "C_deviceType", j, false);
        JsonSerialize(deviceSn, "D_deviceSn", j, false);
        JsonSerialize(deviceEsn, "E_deviceEsn", j, false);
        JsonSerialize(nic, "F_nic", j, false);
        JsonSerialize(mecIp, "G_mecIp", j, false);
        JsonSerialize(regionID, "H_regionID", j, false);
        JsonSerialize(crossID, "I_crossID", j, false);
        JsonSerialize(rsuEuhtIp, "J_rsuEuhtIp", j, false);
        JsonSerialize(rsuEuhtPort, "K_rsuEuhtPort", j, false);
        JsonSerialize(xmlMapFile, "L_xmlMapFile", j, false);
        JsonSerialize(rsiConfigFile, "M_rsiConfigFile", j, false);
        JsonSerialize(rcuid, "N_rcuid", j, false);

        JsonSerialize(senseCloudIp, "O_senseCloudIp", j, false);
        JsonSerialize(senseCloudPort, "P_senseCloudPort", j, false);
        JsonSerialize(senseCaCertName, "Q_senseCaCertName", j, false);
        JsonSerialize(senseClientCertName, "R_senseClientCertName", j, false);
        JsonSerialize(senseClientPrivateKeyFile, "S_senseClientPrivateKeyFile", j, false);
        JsonSerialize(senseClientPrivateKeyPwd, "T_senseClientPrivateKeyPwd", j, false);

        JsonSerialize(maintenanceCloudUrl, "U_maintenanceCloudUrl", j, false);
        JsonSerialize(maintenanceMqttUsername, "V_maintenanceMqttUsername", j, false);
        JsonSerialize(maintenanceMqttPasswd, "W_maintenanceMqttPasswd", j, false);
        JsonSerialize(maintenanceCaCertName, "X_maintenanceCaCertName", j, false);
        JsonSerialize(maintenanceClientCertName, "Y_maintenanceClientCertName", j, false);
        JsonSerialize(maintenanceClientPrivateKeyFile, "Z_maintenanceClientPrivateKeyFile", j, false);
        JsonSerialize(maintenanceClientPrivateKeyPwd, "a_maintenanceClientPrivateKeyPwd", j, false);

        JsonSerialize(videoFtpUrl, "b_videoFtpUrl", j, false);
        JsonSerialize(videoHttpHost, "c_videoHttpHost", j, false);
        JsonSerialize(videoHttpPort, "d_videoHttpPort", j, false);

        JsonSerialize(mecLongitude, "e_mecLongitude", j, false);
        JsonSerialize(mecLatitude, "f_mecLatitude", j, false);
        JsonSerialize(mecAltitude, "g_mecAltitude", j, false);

        JsonSerialize(radarPointCloudFtpIp, "h_radarPointCloudFtpIp", j, false);
        JsonSerialize(radarPointCloudFtpUser, "i_radarPointCloudFtpUser", j, false);
        JsonSerialize(radarPointCloudFtpPasswd, "j_radarPointCloudFtpPasswd", j, false);

        JsonSerialize(ccindexCloudUrl, "k_ccindexCloudUrl", j, false);
        JsonSerialize(ccindexMqttUsername, "l_ccindexMqttUsername", j, false);
        JsonSerialize(ccindexMqttPasswd, "m_ccindexMqttPasswd", j, false);
        JsonSerialize(ccindexCaCertName, "n_ccindexCaCertName", j, false);
        JsonSerialize(ccindexClientCertName, "o_ccindexClientCertName", j, false);
        JsonSerialize(ccindexClientPrivateKeyFile, "p_ccindexClientPrivateKeyFile", j, false);
        JsonSerialize(ccindexClientPrivateKeyPwd, "q_ccindexClientPrivateKeyPwd", j, false);

        JsonSerialize(lightEsn, "r_lightEsn", j, false);
        JsonSerialize(euhtEsn, "s_euhtEsn", j, false);
        JsonSerialize(radarPointCloudFtpPort, "t_radarPointCloudFtpPort", j, false);
        JsonSerialize(license, "u_license", j, false);
    }

    virtual void deserialize(const json &j) override {
        JsonDeserialize(serialNumber, "A_serialNumber", j, noUse_isEmptyFlag);
        JsonDeserialize(routeId, "B_routeId", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceType, "C_deviceType", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceSn, "D_deviceSn", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceEsn, "E_deviceEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(nic, "F_nic", j, noUse_isEmptyFlag);
        JsonDeserialize(mecIp, "G_mecIp", j, noUse_isEmptyFlag);
        JsonDeserialize(regionID, "H_regionID", j, noUse_isEmptyFlag);
        JsonDeserialize(crossID, "I_crossID", j, noUse_isEmptyFlag);
        JsonDeserialize(rsuEuhtIp, "J_rsuEuhtIp", j, noUse_isEmptyFlag);
        JsonDeserialize(rsuEuhtPort, "K_rsuEuhtPort", j, noUse_isEmptyFlag);
        JsonDeserialize(xmlMapFile, "L_xmlMapFile", j, noUse_isEmptyFlag);
        JsonDeserialize(rsiConfigFile, "M_rsiConfigFile", j, noUse_isEmptyFlag);
        JsonDeserialize(rcuid, "N_rcuid", j, noUse_isEmptyFlag);

        JsonDeserialize(senseCloudIp, "O_senseCloudIp", j, noUse_isEmptyFlag);
        JsonDeserialize(senseCloudPort, "P_senseCloudPort", j, noUse_isEmptyFlag);
        JsonDeserialize(senseCaCertName, "Q_senseCaCertName", j, noUse_isEmptyFlag);
        JsonDeserialize(senseClientCertName, "R_senseClientCertName", j, noUse_isEmptyFlag);
        JsonDeserialize(senseClientPrivateKeyFile, "S_senseClientPrivateKeyFile", j, noUse_isEmptyFlag);
        JsonDeserialize(senseClientPrivateKeyPwd, "T_senseClientPrivateKeyPwd", j, noUse_isEmptyFlag);

        JsonDeserialize(maintenanceCloudUrl, "U_maintenanceCloudUrl", j, noUse_isEmptyFlag);
        JsonDeserialize(maintenanceMqttUsername, "V_maintenanceMqttUsername", j, noUse_isEmptyFlag);
        JsonDeserialize(maintenanceMqttPasswd, "W_maintenanceMqttPasswd", j, noUse_isEmptyFlag);
        JsonDeserialize(maintenanceCaCertName, "X_maintenanceCaCertName", j, noUse_isEmptyFlag);
        JsonDeserialize(maintenanceClientCertName, "Y_maintenanceClientCertName", j, noUse_isEmptyFlag);
        JsonDeserialize(maintenanceClientPrivateKeyFile, "Z_maintenanceClientPrivateKeyFile", j,
                        noUse_isEmptyFlag);
        JsonDeserialize(maintenanceClientPrivateKeyPwd, "a_maintenanceClientPrivateKeyPwd", j,
                        noUse_isEmptyFlag);

        JsonDeserialize(videoFtpUrl, "b_videoFtpUrl", j, noUse_isEmptyFlag);
        JsonDeserialize(videoHttpHost, "c_videoHttpHost", j, noUse_isEmptyFlag);
        JsonDeserialize(videoHttpPort, "d_videoHttpPort", j, noUse_isEmptyFlag);

        JsonDeserialize(mecLongitude, "e_mecLongitude", j, noUse_isEmptyFlag);
        JsonDeserialize(mecLatitude, "f_mecLatitude", j, noUse_isEmptyFlag);
        JsonDeserialize(mecAltitude, "g_mecAltitude", j, noUse_isEmptyFlag);

        JsonDeserialize(radarPointCloudFtpIp, "h_radarPointCloudFtpIp", j, noUse_isEmptyFlag);
        JsonDeserialize(radarPointCloudFtpUser, "i_radarPointCloudFtpUser", j, noUse_isEmptyFlag);
        JsonDeserialize(radarPointCloudFtpPasswd, "j_radarPointCloudFtpPasswd", j, noUse_isEmptyFlag);

        JsonDeserialize(ccindexCloudUrl, "k_ccindexCloudUrl", j, noUse_isEmptyFlag);
        JsonDeserialize(ccindexMqttUsername, "l_ccindexMqttUsername", j, noUse_isEmptyFlag);
        JsonDeserialize(ccindexMqttPasswd, "m_ccindexMqttPasswd", j, noUse_isEmptyFlag);
        JsonDeserialize(ccindexCaCertName, "n_ccindexCaCertName", j, noUse_isEmptyFlag);
        JsonDeserialize(ccindexClientCertName, "o_ccindexClientCertName", j, noUse_isEmptyFlag);
        JsonDeserialize(ccindexClientPrivateKeyFile, "p_ccindexClientPrivateKeyFile", j, noUse_isEmptyFlag);
        JsonDeserialize(ccindexClientPrivateKeyPwd, "q_ccindexClientPrivateKeyPwd", j, noUse_isEmptyFlag);

        JsonDeserialize(lightEsn, "r_lightEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(euhtEsn, "s_euhtEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(radarPointCloudFtpPort, "t_radarPointCloudFtpPort", j, noUse_isEmptyFlag);
        JsonDeserialize(license, "u_license", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "serialNumber: " << serialNumber << std::endl;
        ss << std::left << std::setw(40) << "routeId: " << routeId << std::endl;
        ss << std::left << std::setw(40) << "deviceType: " << deviceType << std::endl;
        ss << std::left << std::setw(40) << "deviceSn: " << deviceSn << std::endl;
        ss << std::left << std::setw(40) << "deviceEsn: " << deviceEsn << std::endl;
        ss << std::left << std::setw(40) << "nic: " << nic << std::endl;
        ss << std::left << std::setw(40) << "mecIp: " << mecIp << std::endl;
        ss << std::left << std::setw(40) << "regionID: " << regionID << std::endl;
        ss << std::left << std::setw(40) << "crossID: " << crossID << std::endl;
        ss << std::left << std::setw(40) << "rsuEuhtIp: " << rsuEuhtIp << std::endl;
        ss << std::left << std::setw(40) << "rsuEuhtPort: " << rsuEuhtPort << std::endl;
        ss << std::left << std::setw(40) << "xmlMapFile: " << xmlMapFile << std::endl;
        ss << std::left << std::setw(40) << "rsiConfigFile: " << rsiConfigFile << std::endl;
        ss << std::left << std::setw(40) << "rcuid: " << rcuid << std::endl;
        ss << std::left << std::setw(40) << "senseCloudIp: " << senseCloudIp << std::endl;
        ss << std::left << std::setw(40) << "senseCloudPort: " << senseCloudPort << std::endl;
        ss << std::left << std::setw(40) << "senseCaCertName: " << senseCaCertName << std::endl;
        ss << std::left << std::setw(40) << "senseClientCertName: " << senseClientCertName << std::endl;
        ss << std::left << std::setw(40) << "senseClientPrivateKeyFile: " << senseClientPrivateKeyFile
           << std::endl;
        ss << std::left << std::setw(40) << "senseClientPrivateKeyPwd: " << senseClientPrivateKeyPwd
           << std::endl;
        ss << std::left << std::setw(40) << "maintenanceCloudUrl: " << maintenanceCloudUrl << std::endl;
        ss << std::left << std::setw(40) << "maintenanceMqttUsername: " << maintenanceMqttUsername << std::endl;
        ss << std::left << std::setw(40) << "maintenanceMqttPasswd: " << maintenanceMqttPasswd << std::endl;
        ss << std::left << std::setw(40) << "maintenanceCaCertName: " << maintenanceCaCertName << std::endl;
        ss << std::left << std::setw(40) << "maintenanceClientCertName: " << maintenanceClientCertName
           << std::endl;
        ss << std::left << std::setw(40) << "maintenanceClientPrivateKeyFile: "
           << maintenanceClientPrivateKeyFile
           << std::endl;
        ss << std::left << std::setw(40) << "maintenanceClientPrivateKeyPwd: "
           << maintenanceClientPrivateKeyPwd
           << std::endl;
        ss << std::left << std::setw(40) << "videoFtpUrl: " << videoFtpUrl << std::endl;
        ss << std::left << std::setw(40) << "videoHttpHost: " << videoHttpHost << std::endl;
        ss << std::left << std::setw(40) << "videoHttpPort: " << videoHttpPort << std::endl;
        ss << std::left << std::setw(40) << "mecLongitude: " << mecLongitude << std::endl;
        ss << std::left << std::setw(40) << "mecLatitude: " << mecLatitude << std::endl;
        ss << std::left << std::setw(40) << "mecAltitude: " << mecAltitude << std::endl;
        ss << std::left << std::setw(40) << "radarPointCloudFtpIp: " << radarPointCloudFtpIp << std::endl;
        ss << std::left << std::setw(40) << "radarPointCloudFtpUser: " << radarPointCloudFtpUser << std::endl;
        ss << std::left << std::setw(40) << "radarPointCloudFtpPasswd: " << radarPointCloudFtpPasswd << std::endl;

        ss << std::left << std::setw(40) << "ccindexCloudUrl: " << ccindexCloudUrl << std::endl;
        ss << std::left << std::setw(40) << "ccindexMqttUsername: " << ccindexMqttUsername << std::endl;
        ss << std::left << std::setw(40) << "ccindexMqttPasswd: " << ccindexMqttPasswd << std::endl;
        ss << std::left << std::setw(40) << "ccindexCaCertName: " << ccindexCaCertName << std::endl;
        ss << std::left << std::setw(40) << "ccindexClientCertName: " << ccindexClientCertName << std::endl;
        ss << std::left << std::setw(40) << "ccindexClientPrivateKeyFile: " << ccindexClientPrivateKeyFile << std::endl;
        ss << std::left << std::setw(40) << "ccindexClientPrivateKeyPwd: " << ccindexClientPrivateKeyPwd << std::endl;

        ss << std::left << std::setw(40) << "lightEsn: " << lightEsn << std::endl;

        ss << std::left << std::setw(40) << "euhtEsn: " << euhtEsn << std::endl;
        ss << std::left << std::setw(40) << "radarPointCloudFtpPort: " << euhtEsn << std::endl;
        ss << std::left << std::setw(40) << "u_license: " << license << std::endl;
        return ss.str();
    }
    // 定义 < 运算符
    bool operator<(const MecDeviceWorkParam& other) const {
        return std::tie(serialNumber, routeId, deviceType, deviceSn, deviceEsn, nic, mecIp, regionID, crossID,
                        rsuEuhtIp, rsuEuhtPort, xmlMapFile, rsiConfigFile, rcuid,
                        senseCloudIp, senseCloudPort, senseCaCertName, senseClientCertName,
                        senseClientPrivateKeyFile, senseClientPrivateKeyPwd,
                        maintenanceCloudUrl, maintenanceMqttUsername, maintenanceMqttPasswd,
                        maintenanceCaCertName, maintenanceClientCertName, maintenanceClientPrivateKeyFile,
                        maintenanceClientPrivateKeyPwd, videoFtpUrl, videoHttpHost, videoHttpPort,
                        mecLongitude, mecLatitude, mecAltitude,
                        radarPointCloudFtpIp, radarPointCloudFtpUser, radarPointCloudFtpPasswd,
                        ccindexCloudUrl, ccindexMqttUsername, ccindexMqttPasswd,
                        ccindexCaCertName, ccindexClientCertName, ccindexClientPrivateKeyFile,
                        ccindexClientPrivateKeyPwd, lightEsn, euhtEsn, radarPointCloudFtpPort) <
               std::tie(other.serialNumber, other.routeId, other.deviceType, other.deviceSn, other.deviceEsn,
                        other.nic, other.mecIp, other.regionID, other.crossID,
                        other.rsuEuhtIp, other.rsuEuhtPort, other.xmlMapFile, other.rsiConfigFile, other.rcuid,
                        other.senseCloudIp, other.senseCloudPort, other.senseCaCertName,
                        other.senseClientCertName, other.senseClientPrivateKeyFile, other.senseClientPrivateKeyPwd,
                        other.maintenanceCloudUrl, other.maintenanceMqttUsername, other.maintenanceMqttPasswd,
                        other.maintenanceCaCertName, other.maintenanceClientCertName,
                        other.maintenanceClientPrivateKeyFile, other.maintenanceClientPrivateKeyPwd,
                        other.videoFtpUrl, other.videoHttpHost, other.videoHttpPort,
                        other.mecLongitude, other.mecLatitude, other.mecAltitude,
                        other.radarPointCloudFtpIp, other.radarPointCloudFtpUser, other.radarPointCloudFtpPasswd,
                        other.ccindexCloudUrl, other.ccindexMqttUsername, other.ccindexMqttPasswd,
                        other.ccindexCaCertName, other.ccindexClientCertName, other.ccindexClientPrivateKeyFile,
                        other.ccindexClientPrivateKeyPwd, other.lightEsn, other.euhtEsn, other.radarPointCloudFtpPort);
    }
};

struct SensorDeviceWorkParam : public afl::base::SerializableData
{
    public:
    int serialNumber = 0; // 序号
    std::string routeId; // 路口编号
    WorkParamDeviceType deviceType; // 设备类型
    std::string mecSn; // MEC-Sn
    std::string deviceSn; // 设备sn
    std::string deviceEsn; // 设备esn
    std::string deviceIp; // 设备IP
    double deviceLongitude = 0; // 设备经度
    double deviceLatitude = 0; // 设备维度
    double deviceAltitude = 0; // 设备海拔
    std::string radarCrossId; // 雷达CrossId
    int radarPort = 0; // 雷达PORT
    std::string vendor;
    std::string category;
    double northAngle;
public:
    virtual void serialize(json &j) override {
        JsonSerialize(serialNumber, "A_serialNumber", j, false);
        JsonSerialize(routeId, "B_routeId", j, false);
        JsonSerialize(deviceType, "C_deviceType", j, false);
        JsonSerialize(mecSn, "D_mecSn", j, false);
        JsonSerialize(deviceSn, "E_deviceSn", j, false);
        JsonSerialize(deviceEsn, "F_deviceEsn", j, false);
        JsonSerialize(deviceIp, "G_deviceIp", j, false);
        JsonSerialize(deviceLongitude, "H_deviceLongitude", j, false);
        JsonSerialize(deviceLatitude, "I_deviceLatitude", j, false);
        JsonSerialize(deviceAltitude, "J_deviceAltitude", j, false);
        JsonSerialize(radarCrossId, "K_radarCrossId", j, false);
        JsonSerialize(radarPort, "L_radarPort", j, false);
        JsonSerialize(vendor, "M_vendor", j, false);
        JsonSerialize(category, "N_category", j, false);
        JsonSerialize(northAngle, "O_northAngle", j, false);
    }

    virtual void deserialize(const json &j) override {
        JsonDeserialize(serialNumber, "A_serialNumber", j, noUse_isEmptyFlag);
        JsonDeserialize(routeId, "B_routeId", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceType, "C_deviceType", j, noUse_isEmptyFlag);
        JsonDeserialize(mecSn, "D_mecSn", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceSn, "E_deviceSn", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceEsn, "F_deviceEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceIp, "G_deviceIp", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceLongitude, "H_deviceLongitude", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceLatitude, "I_deviceLatitude", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceAltitude, "J_deviceAltitude", j, noUse_isEmptyFlag);
        JsonDeserialize(radarCrossId, "K_radarCrossId", j, noUse_isEmptyFlag);
        JsonDeserialize(radarPort, "L_radarPort", j, noUse_isEmptyFlag);
        JsonDeserialize(vendor, "M_vendor", j, noUse_isEmptyFlag);
        JsonDeserialize(category, "N_category", j, noUse_isEmptyFlag);

        JsonDeserialize(northAngle, "O_northAngle", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const {
        std::stringstream ss;
        if(!deviceEsn.empty())
        {
            ss << std::left << std::setw(40) << "Serial Number: " << serialNumber << std::endl;
            ss << std::left << std::setw(40) << "Route Id: " << routeId << std::endl;
            ss << std::left << std::setw(40) << "Device Type: " << deviceType << std::endl;
            ss << std::left << std::setw(40) << "MEC Sn: " << mecSn << std::endl;
            ss << std::left << std::setw(40) << "Device Sn: " << deviceSn << std::endl;
            ss << std::left << std::setw(40) << "Device ESN: " << deviceEsn << std::endl;
            ss << std::left << std::setw(40) << "Device IP: " << deviceIp << std::endl;
            ss << std::left << std::setw(40) << "Device Longitude: " << deviceLongitude << std::endl;
            ss << std::left << std::setw(40) << "Device Latitude: " << deviceLatitude << std::endl;
            ss << std::left << std::setw(40) << "Device Altitude: " << deviceAltitude << std::endl;
            ss << std::left << std::setw(40) << "Radar Cross Id: " << radarCrossId << std::endl;
            ss << std::left << std::setw(40) << "Radar Port: " << radarPort << std::endl;
            ss << std::left << std::setw(40) << "Vendor: " << vendor << std::endl;
            ss << std::left << std::setw(40) << "Category: " << category << std::endl;

            ss << std::left << std::setw(40) << "NorthAngle: " << northAngle << std::endl;
        }
        return ss.str();
    }

    bool operator<(const SensorDeviceWorkParam& other) const {
        return std::tie(serialNumber, routeId, deviceType, mecSn, deviceSn, deviceEsn, deviceIp,
                        deviceLongitude, deviceLatitude, deviceAltitude, radarCrossId,
                        radarPort, vendor, category) <
               std::tie(other.serialNumber, other.routeId, other.deviceType, other.mecSn,
                        other.deviceSn, other.deviceEsn, other.deviceIp,
                        other.deviceLongitude, other.deviceLatitude, other.deviceAltitude,
                        other.radarCrossId, other.radarPort, other.vendor, other.category);
    }
};

struct CCInexLaneWorkParam : public afl::base::SerializableData
{
    public:
    std::string crossid = ""; 	//路口id
    std::string branchId = ""; //路口分支id
    std::string laneId = ""; //车道id
    int phaseId; //车流行驶方向对应相位id
    std::string meclaneId = "";//mec2011接口中对应得laneid
public:
    virtual void serialize(json &j) override {
        JsonSerialize(crossid, "A_crossid", j, false);
        JsonSerialize(branchId, "B_branchId", j, false);
        JsonSerialize(laneId, "C_laneId", j, false);
        JsonSerialize(phaseId, "D_phaseId", j, false);
        JsonSerialize(meclaneId, "E_meclaneId", j, false);
        
    }

    virtual void deserialize(const json &j) override {
        JsonDeserialize(crossid, "A_crossid", j, noUse_isEmptyFlag);
        JsonDeserialize(branchId, "B_branchId", j, noUse_isEmptyFlag);
        JsonDeserialize(laneId, "C_laneId", j, noUse_isEmptyFlag);
        JsonDeserialize(phaseId, "D_phaseId", j, noUse_isEmptyFlag);
        JsonDeserialize(meclaneId, "E_meclaneId", j, noUse_isEmptyFlag);

    }

public:
    std::string to_string() const {
        std::stringstream ss;
        if(!laneId.empty())
        {
            ss << std::left << std::setw(40) << "crossid: " << crossid << std::endl;
            ss << std::left << std::setw(40) << "branchId: " << branchId << std::endl;
            ss << std::left << std::setw(40) << "laneId: " << laneId << std::endl;
            ss << std::left << std::setw(40) << "phaseId: " << phaseId << std::endl;
            ss << std::left << std::setw(40) << "meclaneId: " << meclaneId << std::endl;

        }
        return ss.str();
    }

    bool operator<(const CCInexLaneWorkParam& other) const {
        return std::tie(crossid, branchId, laneId, phaseId) <
               std::tie(other.crossid, other.branchId, other.laneId, other.phaseId);
    }
};
struct CCInexFlowWorkParam : public afl::base::SerializableData
{
    public:
    std::string crossid = ""; //路口id
    std::string branchId= "";	//路口分支id
    int turnType; //车流行驶方向：1直行  2左转  3右转  4掉头
    int phaseId; //车流行驶方向对应相位id
public:
    virtual void serialize(json &j) override {
        JsonSerialize(crossid, "A_crossid", j, false);
        JsonSerialize(branchId, "B_branchId", j, false);
        JsonSerialize(turnType, "C_turnType", j, false);
        JsonSerialize(phaseId, "D_phaseId", j, false);
    }

    virtual void deserialize(const json &j) override {
        JsonDeserialize(crossid, "A_crossid", j, noUse_isEmptyFlag);
        JsonDeserialize(branchId, "B_branchId", j, noUse_isEmptyFlag);
        JsonDeserialize(turnType, "C_turnType", j, noUse_isEmptyFlag);
        JsonDeserialize(phaseId, "D_phaseId", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const {
        std::stringstream ss;
        if(!branchId.empty())
        {
            ss << std::left << std::setw(40) << "A_crossid: " << crossid << std::endl;
            ss << std::left << std::setw(40) << "B_branchId: " << branchId << std::endl;
            ss << std::left << std::setw(40) << "C_turnType: " << turnType << std::endl;
            ss << std::left << std::setw(40) << "D_phaseId: " << phaseId << std::endl;
        }
        return ss.str();
    }

    bool operator<(const CCInexFlowWorkParam& other) const {
        return std::tie(crossid, branchId, turnType, phaseId) <
               std::tie(other.crossid, other.branchId, other.turnType, other.phaseId);
    }
    
};

struct CCInexWorkParam : public afl::base::SerializableData
{
public:
    std::vector<CCInexLaneWorkParam> laneList;
    std::vector<CCInexFlowWorkParam> flowList;
    CCInexWorkParam()
    {
        for(uint8_t i = 0;  i < 10; i++)
        {
            CCInexLaneWorkParam ccInexLaneWorkParam;
            CCInexFlowWorkParam ccInexFlowWorkParam;
            laneList.push_back(ccInexLaneWorkParam);
            flowList.push_back(ccInexFlowWorkParam);
        }
    }

private:
    virtual void serialize(json &j) override {

        JsonSerialize(laneList, "A_laneList", j, false);
        JsonSerialize(flowList, "B_flowList", j, false);
    }

    virtual void deserialize(const json &j) override {
        JsonDeserialize(laneList, "A_laneList", j, noUse_isEmptyFlag);
        JsonDeserialize(flowList, "B_flowList", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Lane List:\n";
        for (auto lane : laneList) {
            if(!lane.to_string().empty())
            {
                ss << lane.to_string() << std::endl;
            }

        }
        ss << std::left << std::setw(40) << "Flow List:\n";
        for (auto flow : flowList) {
            if(!flow.to_string().empty())
            {
                ss << flow.to_string() << std::endl;
            }
        }
        return ss.str();
    }
public:
    
    bool operator<(const CCInexWorkParam& other) const {
        // 按顺序比较 laneList 和 flowList
        if (laneList < other.laneList) {
            return true;  // 如果当前的 laneList 小于其他的，则当前对象小于其他对象
        }
        if (other.laneList < laneList) {
            return false; // 如果当前的 laneList 大于其他的，则当前对象不小于其他对象
        }
        // 如果 laneList 相等，则比较 flowList
        return flowList < other.flowList;
    }
};

//////////////////////////////////////////////////////////////////////
struct OmPtpLogParamConfiger : public afl::base::SerializableData
{
public:
    
    std::string localSshUserName = "t";
    std::string localSshPassword = "11111111";
    std::string ftpUserName = "ftpuser";
    std::string ftpPassword = "password";
    int       ftpPort = 22222;
    std::string ftpDir = "/home/airos";

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(localSshUserName, "A_localSshUserName", j, false);
        JsonSerialize(localSshPassword, "B_localSsshPassword", j, false);
        JsonSerialize(ftpUserName, "C_ftpUserName", j, false);
        JsonSerialize(ftpPassword, "D_ftpPassword", j, false);
        JsonSerialize(ftpPort, "E_ftpPort", j, false);
        JsonSerialize(ftpDir, "F_ftpDir", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(localSshUserName, "A_localSshUserName", j, noUse_isEmptyFlag);
        JsonDeserialize(localSshPassword, "B_localSshPassword", j, noUse_isEmptyFlag);
        JsonDeserialize(ftpUserName, "C_ftpUserName", j, noUse_isEmptyFlag);
        JsonDeserialize(ftpPassword, "D_ftpPassword", j, noUse_isEmptyFlag);
        JsonDeserialize(ftpPort, "E_ftpPort", j, noUse_isEmptyFlag);
        JsonDeserialize(ftpDir, "F_ftpDir", j, noUse_isEmptyFlag);
    }

    public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_localSshUserName: " << localSshUserName << std::endl;
        ss << std::left << std::setw(40) << "B_localSshPassword:" << localSshPassword << std::endl;
        ss << std::left << std::setw(40) << "C_ftpUserName:" << ftpUserName << std::endl;
        ss << std::left << std::setw(40) << "D_ftpPassword:" << ftpPassword << std::endl;
        ss << std::left << std::setw(40) << "E_ftpPort:" << ftpPort << std::endl;
        ss << std::left << std::setw(40) << "F_ftpDir:" << ftpDir << std::endl;
        return ss.str();
    }
    bool operator<(const OmPtpLogParamConfiger& other) const {
        return std::tie(localSshUserName, localSshPassword, ftpUserName, ftpPassword, ftpPort, ftpDir) <
               std::tie(other.localSshUserName, other.localSshPassword, other.ftpUserName, other.ftpPassword, other.ftpPort, other.ftpDir);
    }
};

//摄像头违法事件功能配置
struct CameraEventParamConfiger : public afl::base::SerializableData
{
//    int httpPort = 40415;
//    int ftpPort = 40416;
//    std::string imageDir = "/home/airos";
//    std::string httpPath = "/camera_event";
    std::string tcpIp = "127.0.0.1";
    int tcpPort = 40414;
private:
    virtual void serialize(json &j) override
    {
      JsonSerialize(tcpIp, "A_tcpIp", j, false);
      JsonSerialize(tcpPort, "B_tcpPort", j, false);
//        JsonSerialize(httpPort, "A_httpPort", j, false);
//        JsonSerialize(ftpPort, "B_ftpPort", j, false);
//        JsonSerialize(imageDir, "C_imageDir", j, false);
//        JsonSerialize(httpPath, "D_httpPath", j, false);


    }

    virtual void deserialize(const json &j) override
    {
      JsonDeserialize(tcpIp, "A_tcpIp", j, noUse_isEmptyFlag);
      JsonDeserialize(tcpPort, "B_tcpPort", j, noUse_isEmptyFlag);
//        JsonDeserialize(httpPort, "E_httpPort", j, noUse_isEmptyFlag);
//        JsonDeserialize(ftpPort, "F_ftpPort", j, noUse_isEmptyFlag);
//        JsonDeserialize(imageDir, "G_imageDir", j, noUse_isEmptyFlag);
//        JsonDeserialize(httpPath, "H_httpPath", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "tcpIp:" << tcpIp << std::endl;
        ss << std::left << std::setw(40) << "tcpPort:" << tcpPort << std::endl;
//        ss << std::left << std::setw(40) << "httpPort: " << httpPort << std::endl;
//        ss << std::left << std::setw(40) << "ftpPort:" << ftpPort << std::endl;
//        ss << std::left << std::setw(40) << "imageDir:" << imageDir << std::endl;
//        ss << std::left << std::setw(40) << "httpPath:" << httpPath << std::endl;
        return ss.str();
    }

    bool operator<(const CameraEventParamConfiger& other) const {
        return std::tie(tcpIp, tcpPort) <
               std::tie(other.tcpIp, other.tcpPort);
    }
};

struct TrafficLightParamConfiger : public afl::base::SerializableData
{
public:
    std::string serverIp = "127.0.0.1";
    int serverPort = 10023;
    std::string peerIp = "127.0.0.1";
    int peerPort = 50807;
    int localPort  =  10050;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(serverIp, "A_serverIp", j, false);
        JsonSerialize(serverPort, "B_serverPort", j, false);
        JsonSerialize(peerIp, "C_peerIp", j, false);
        JsonSerialize(peerPort, "D_peerPort", j, false);
        JsonSerialize(localPort, "E_localPort", j, false);
        
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(serverIp, "A_serverIp", j, noUse_isEmptyFlag);
        JsonDeserialize(serverPort, "B_serverPort", j, noUse_isEmptyFlag);
        JsonDeserialize(peerIp, "C_peerIp", j, noUse_isEmptyFlag);
        JsonDeserialize(peerPort, "D_peerPort", j, noUse_isEmptyFlag);
        JsonDeserialize(localPort, "E_localPort", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "serverIp:" << serverIp << std::endl;
        ss << std::left << std::setw(40) << "serverPort:" << serverPort << std::endl;
        ss << std::left << std::setw(40) << "peerIp:" << peerIp << std::endl;
        ss << std::left << std::setw(40) << "peerPort:" << peerPort << std::endl;
        ss << std::left << std::setw(40) << "localPort:" << localPort << std::endl;
        return ss.str();
    }

    bool operator<(const TrafficLightParamConfiger& other) const {
        return std::tie(serverIp, serverPort, peerIp, peerPort, localPort) <
               std::tie(other.serverIp, other.serverPort, other.peerIp, other.peerPort, other.localPort);
    }
};


struct RadarStaticHttpConfiger : public afl::base::SerializableData
{
    //本机作为server
    std::string httpHostServerIp = "0.0.0.0";
    uint16_t    httpHostServerPort = 8080;
    std::string httpCloudClientGetPath = "/static/device/config";
    uint16_t    mecMqttClientQueryConfigDataPeriod = 60;  //mqtt-client 推送查询数据周期
    //////////////////////////
    //本机作为client
    std::string httpCloudServerIp = "172.20.65.184";
    uint16_t    httpCloudServerPort = 8080;
    uint16_t    httpHostClientPostConfigUpdateDataPeriod = 300;  //http-client-post方法推送更新数据周期

    virtual void serialize(json &j) override
    {
        JsonSerialize(httpHostServerIp, "A_httpHostServerIp", j, false);
        JsonSerialize(httpHostServerPort, "B_httpHostServerPort", j, false);
        JsonSerialize(httpCloudClientGetPath, "C_httpCloudClientGetPath", j, false);
        JsonSerialize(mecMqttClientQueryConfigDataPeriod, "D_mecMqttClientQueryConfigDataPeriod", j, false);
        JsonSerialize(httpCloudServerIp, "E_httpCloudServerIp", j, false);
        JsonSerialize(httpCloudServerPort, "F_httpCloudServerPort", j, false);
        JsonSerialize(httpHostClientPostConfigUpdateDataPeriod, "G_httpHostClientPostConfigUpdateDataPeriod", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(httpHostServerIp, "A_httpHostServerIp", j, noUse_isEmptyFlag);
        JsonDeserialize(httpHostServerPort, "B_httpHostServerPort", j, noUse_isEmptyFlag);
        JsonDeserialize(httpCloudClientGetPath, "C_httpCloudClientGetPath", j, noUse_isEmptyFlag);
        JsonDeserialize(mecMqttClientQueryConfigDataPeriod, "D_mecMqttClientQueryConfigDataPeriod", j, noUse_isEmptyFlag);
        JsonDeserialize(httpCloudServerIp, "E_httpCloudServerIp", j, noUse_isEmptyFlag);
        JsonDeserialize(httpCloudServerPort, "F_httpCloudServerPort", j, noUse_isEmptyFlag);
        JsonDeserialize(httpHostClientPostConfigUpdateDataPeriod, "G_httpHostClientPostConfigUpdateDataPeriod", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;

        ss << std::left << std::setw(40) << "httpHostServerIp: " << httpHostServerIp << std::endl;
        ss << std::left << std::setw(40) << "httpHostServerPort: " << httpHostServerPort << std::endl;
        ss << std::left << std::setw(40) << "httpCloudClientGetPath: " << httpCloudClientGetPath << std::endl;
        ss << std::left << std::setw(40) << "mecMqttClientQueryConfigDataPeriod: " << mecMqttClientQueryConfigDataPeriod << std::endl;
        ss << std::left << std::setw(40) << "httpCloudServerIp: " << httpCloudServerIp << std::endl;
        ss << std::left << std::setw(40) << "httpCloudServerPort: " << httpCloudServerPort << std::endl;
        ss << std::left << std::setw(40) << "httpHostClientPostConfigUpdateDataPeriod: " << httpHostClientPostConfigUpdateDataPeriod << std::endl;
        return ss.str();
    }
};


//rsu客户端配置
struct OmWorkParamConfiger : public afl::base::SerializableData
{
public:

    bool hasedInit = false;
    MecDeviceWorkParam mecDeviceWorkParam;
    int sensorDeviceNum = 0;
    std::vector<SensorDeviceWorkParam> sensorDeviceWorkParamList;
    CCInexWorkParam ccInexWorkParam;
    OmPtpLogParamConfiger omPtpLogParamConfiger;
    CameraEventParamConfiger cameraEventParamConfiger;
    TrafficLightParamConfiger trafficLightParamConfiger;
//    RadarStaticHttpConfiger radarStaticHttpConfiger;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(hasedInit, "A_hasedInit", j, false);
        JsonSerialize(mecDeviceWorkParam, "B_mecDeviceWorkParam", j, false);
        JsonSerialize(sensorDeviceNum, "C_sensorDeviceNum", j, false);
        JsonSerialize(sensorDeviceWorkParamList, "D_sensorDeviceWorkParamList", j, false);
        JsonSerialize(ccInexWorkParam, "E_ccInexWorkParam", j, false);
        JsonSerialize(omPtpLogParamConfiger, "F_ptpLogParam", j, false);
        JsonSerialize(cameraEventParamConfiger, "G_cameraEventParam", j, false);
        JsonSerialize(trafficLightParamConfiger, "H_trafficLightParam", j, false);
        //        JsonSerialize(radarStaticHttpConfiger, "H_radarStaticHttpConfiger", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(hasedInit, "A_hasedInit", j, noUse_isEmptyFlag);
        JsonDeserialize(mecDeviceWorkParam, "B_mecDeviceWorkParam", j, noUse_isEmptyFlag);
        JsonDeserialize(sensorDeviceNum, "C_sensorDeviceNum", j, noUse_isEmptyFlag);
        JsonDeserialize(sensorDeviceWorkParamList, "D_sensorDeviceWorkParamList", j, noUse_isEmptyFlag);
        JsonDeserialize(ccInexWorkParam, "E_ccInexWorkParam", j, noUse_isEmptyFlag);
        JsonDeserialize(omPtpLogParamConfiger, "F_ptpLogParam", j, noUse_isEmptyFlag);
        JsonDeserialize(cameraEventParamConfiger, "G_cameraEventParam", j, noUse_isEmptyFlag);
        JsonDeserialize(trafficLightParamConfiger, "H_trafficLightParam", j, noUse_isEmptyFlag);
        //        JsonDeserialize(radarStaticHttpConfiger, "H_radarStaticHttpConfiger", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "HasedInit: " << (hasedInit ? "True" : "False") << std::endl;
        ss << std::left << std::setw(40) << "MecDeviceWorkParam:" << std::endl << mecDeviceWorkParam.to_string() << std::endl;
        ss << std::left << std::setw(40) << "sensorDeviceNum:" << std::endl << sensorDeviceNum << std::endl;
        ss << std::left << std::setw(40) << "SensorDeviceWorkParamList:" << std::endl;

        for (auto param: sensorDeviceWorkParamList)
        {
            if(!param.to_string().empty())
            {
                ss << param.to_string() << std::endl;
            }
        }
        ss << std::left << std::setw(40) << "CCInexWorkParam:" << std::endl;
        ss << ccInexWorkParam.to_string() << std::endl;
        ss << std::left << std::setw(40) << "OmPtpLogParamConfiger:" << omPtpLogParamConfiger.to_string()<< std::endl;
        ss << std::left << std::setw(40) << "cameraEventParamConfiger:" << cameraEventParamConfiger.to_string()<< std::endl;
        ss << std::left << std::setw(40) << "trafficLightParamConfiger:" << trafficLightParamConfiger.to_string()<< std::endl;
        //        ss << std::left << std::setw(40) << "radarStaticHttpConfiger:" << radarStaticHttpConfiger.to_string()<< std::endl;
        return ss.str();
    }
    bool operator<(const OmWorkParamConfiger& other) const {
        return std::tie(hasedInit, mecDeviceWorkParam, sensorDeviceNum, sensorDeviceWorkParamList, ccInexWorkParam, omPtpLogParamConfiger, cameraEventParamConfiger, trafficLightParamConfiger) <
               std::tie(other.hasedInit, other.mecDeviceWorkParam, other.sensorDeviceNum, other.sensorDeviceWorkParamList,
                        other.ccInexWorkParam, other.omPtpLogParamConfiger, other.cameraEventParamConfiger, other.trafficLightParamConfiger);
    }
};

class WorkParam {
public:
    WorkParam() = default;

    virtual ~WorkParam();

    static bool
    getWorkParamFromFile(std::string workParamFilePath, OmWorkParamConfiger &omWorkParamConfiger) {
//        ERRORPRINT("[work-param-file-path]%s", workParamFilePath.c_str());
        std::ifstream file(workParamFilePath);
        if (!file) {
            ERRORPRINT("[error]Failed to open file!");
            return false;
        }
        afl::base::json workParamJson;
        try {
            file >> workParamJson;
            omWorkParamConfiger = workParamJson;
        }
        catch (afl::base::json::exception &e) {
            ERRORPRINT("[error]%s", e.what());
            return false;
        }
        return true;
    }

    static OmWorkParamConfiger
    getWorkParamFromFile(std::string workParamFilePath = "/home/airos/common_config/work_param_config.flag")
    {

        OmWorkParamConfiger omWorkParamConfiger;
        std::ifstream file(workParamFilePath);
        if (!file){
            ERRORPRINT("[error]Failed to open file!");
        }
        afl::base::json workParamJson;
        try {
            file >> workParamJson;
            omWorkParamConfiger = workParamJson;
        }
        catch (afl::base::json::exception &e) {
            ERRORPRINT("[error]%s", e.what());
        }
        return omWorkParamConfiger;
    }
    static bool updateWorkParamFile(const std::string& workParamFilePath, OmWorkParamConfiger& omWorkParamConfiger)
    {

        afl::base::json workParamJson;
        try {
            workParamJson = omWorkParamConfiger;
        }
        catch (afl::base::json::exception &e) {
            ERRORPRINT("[error]%s", e.what());
        }

        // 将修改后的 JSON 对象写回到文件
        std::ofstream file(workParamFilePath);
        if (!file) {
            ERRORPRINT("[error]Failed to open file for writing!");
            return false;
        }

        file << workParamJson.dump(4); // 使用 4 个空格缩进进行格式化输出
        file.close();

        return true;
    }
};
//////////////////////////////////////////////////////////////////////////////
}

}

}
#endif //AIROS2_0_NEW_CONFIGER_OM_WORK_PARAM_H
