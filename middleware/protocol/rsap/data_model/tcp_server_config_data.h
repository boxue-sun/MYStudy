/*
 * @Author: zhangenwei
 * @Date: 2024-02-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef MIDDLEWART_DEVICE_SERVICE_RSAP_DATA_MODEL_RSAP_CONFIG
#define MIDDLEWART_DEVICE_SERVICE_RSAP_DATA_MODEL_RSAP_CONFIG
#include "namespace.h"
#include <string>
NAMESPACE_PROTOCOL_THREAD_START
//数据包结构
struct CloudServiceConfig
{
    std::string serverIP;
    uint16_t    serverPort;
    uint16_t    localPort;
    double     heartBeatPeriod;
    double     heartBeatMonitorPeriod = 2;
    double     heartBeatReconnectTimeMinute = 3;
    bool       enbaleHeartBeatReconnect = true;

    double     deviceStatusPeriod;
    double     deviceStatusMonitorPeriod = 2;
    double     deviceStatusReconnectTimeMinute = 3;
    bool       enbaleDeviceStatusReconnect = true;
    double     deviceStatusDelPeriod;

    double     eventMonitorPeriod = 2;
    double     eventReconnectTimeMinute = 3;
    bool       enbaleEventReconnect = true;

    std::string protocol;
    std::string objRcuId;
    std::string objDeviceId;
    uint16_t objDeviceType;


    std::string projectRoot = "/home/airos/protocl/rsap/";
    std::string tlsRoot = "tls/";
    std::string tlsCaFileName = "ca.crt";
    std::string tlsClientKeyFileName = "U-XA000M.crt";
    std::string tlsClientPrivateKeyFileName = "U-XA000M_pkcs8.key";
    std::string tlsClientPrivateKeyPasswd = "123456";
    bool enabelUseTls = false;


    bool enablePushHeartbeat = true;
    bool enablePushObj = true;
    bool enablePushEvent = true;
    bool enablePushStatus = true;

    bool enableScribeHeartbeat = true;
    bool enableScribeObj = true;
    bool enableScribeEvent = true;
    bool enableScribeStatus = true;

    bool enablePrintParseInfo = true;
    bool enablePrintInfo = true;
    bool enablePrintInfoLog = true;
    bool enable_print_sensor_channel_data_info = false;
    bool enable_use_real_dev_no = false;
    bool enable_use_camera_no_add_01_as_hash_compute = false;
    bool enableUseSystime = true;
    bool enableUse02 = true;
    bool enableUseAddUtc8 = true;
    bool enablePrintParseInfoHeartbeat = true;
    bool enablePrintParseInfoObj = true;
    bool enablePrintParseInfoEvent = true;
    bool enablePrintParseInfoStatus = true;
    std::string deviceStatusDbPath;
    std::string deviceStatusDbTableName;
    std::string confDirName;

    std::string deviceNoMapFileName;
    std::string deviceNoMapFilePath;
    std::string workParamFilePath;
    std::string to_string() const
    {
        std::ostringstream oss;
        oss << "serverIP: " << serverIP << std::endl
            << "serverPort: " << serverPort << std::endl
            << "localPort: " << localPort << std::endl
            << "heartBeatPeriod: " << heartBeatPeriod << std::endl
            << "heartBeatMonitorPeriod: " << heartBeatMonitorPeriod << std::endl
            << "protocol: " << protocol << std::endl
            << "objRcuId: " << objRcuId << std::endl
            << "objDeviceId: " << objDeviceId << std::endl
            << "objDeviceType: " << objDeviceType << std::endl
            << "projectRoot: " << projectRoot << std::endl
            << "tlsRoot: " << tlsRoot << std::endl
            << "tlsCaFileName: " << tlsCaFileName << std::endl
            << "tlsClientKeyFileName: " << tlsClientKeyFileName << std::endl
            << "tlsClientPrivateKeyFileName: " << tlsClientPrivateKeyFileName << std::endl
            << "tlsClientPrivateKeyPasswd: " << tlsClientPrivateKeyPasswd << std::endl
            << "enabelUseTls: " << (enabelUseTls ? "true" : "false") << std::endl
            << "enbaleHeartBeatReconnect: " << (enbaleHeartBeatReconnect ? "true" : "false") << std::endl
            << "enablePushHeartbeat: " << (enablePushHeartbeat ? "true" : "false") << std::endl
            << "enablePushObj: " << (enablePushObj ? "true" : "false") << std::endl
            << "enablePushEvent: " << (enablePushEvent ? "true" : "false") << std::endl
            << "enablePushStatus: " << (enablePushStatus ? "true" : "false") << std::endl
            << "enableScribeHeartbeat: " << (enableScribeHeartbeat ? "true" : "false") << std::endl
            << "enableScribeObj: " << (enableScribeObj ? "true" : "false") << std::endl
            << "enableScribeEvent: " << (enableScribeEvent ? "true" : "false") << std::endl
            << "enableScribeStatus: " << (enableScribeStatus ? "true" : "false") << std::endl
            << "enablePrintParseInfo: " << (enablePrintParseInfo ? "true" : "false") << std::endl
            << "enablePrintInfo: " << (enablePrintInfo ? "true" : "false") << std::endl
            << "enablePrintInfoLog: " << (enablePrintInfoLog ? "true" : "false") << std::endl
            << "enable_print_sensor_channel_data_info: " << (enable_print_sensor_channel_data_info ? "true" : "false") << std::endl
            << "enable_use_real_dev_no: " << (enable_use_real_dev_no ? "true" : "false") << std::endl
            << "enableUseSystime: " << (enableUseSystime ? "true" : "false") << std::endl
            << "enableUse02: " << (enableUse02 ? "true" : "false") << std::endl
            << "enableUseAddUtc8: " << (enableUseAddUtc8 ? "true" : "false") << std::endl
            << "enablePrintParseInfoHeartbeat: " << (enablePrintParseInfoHeartbeat ? "true" : "false") << std::endl
            << "enablePrintParseInfoObj: " << (enablePrintParseInfoObj ? "true" : "false") << std::endl
            << "enablePrintParseInfoEvent: " << (enablePrintParseInfoEvent ? "true" : "false") << std::endl
            << "enablePrintParseInfoStatus: " << (enablePrintParseInfoStatus ? "true" : "false") << std::endl
            << "deviceStatusDbPath: " << deviceStatusDbPath << std::endl
            << "deviceStatusDbTableName: " << deviceStatusDbTableName << std::endl
            << "deviceNoMapFileName: " << deviceNoMapFileName << std::endl
            << "deviceNoMapFilePath: " << deviceNoMapFilePath << std::endl
            << "workParamFilePath: " << workParamFilePath << std::endl
            ;

        return oss.str();
    }

};
NAMESPACE_PROTOCOL_THREAD_END

#endif