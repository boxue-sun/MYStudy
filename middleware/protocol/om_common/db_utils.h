/*********************************************************************************
* @file		    db_utils.h
* @brief		db_utils.h belongs to CICTCI
* @details
* @author		alfred
* @email        zhangenwei64@gmail.com
* @date		    24-8-24
* @copyright	Copyright (c) 2024 Mec-Airos Division.
* @verbatim
*
*  Change History:
*  Date      Author    Version  ChangeId           Description
*  ------------------------------------------------------------------------------
*  24-8-24 alfred       1.0       ————             Create this file
*
* @endverbatim
********************************************************************************/
#ifndef AIROS2_0_OM_COMMON_DB_UTILS_H
#define AIROS2_0_OM_COMMON_DB_UTILS_H
#include "namespace.h"
#include "base/common/network/exception.h"
#include "data_model/data_common.h"
#include "sqlite_device_status.h"
#include <string>   // 用于 std::string
#include <regex>    // 用于 std::regex 和 std::smatch
#include <iostream> // (如果需要) 用于 std::cout 或 std::cerr
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <memory>
#include <thread> // 用于 sleep_for
#include <chrono> // 用于高精度时间
NAMESPACE_START_OM_COMPONENT_COMMON
using namespace std;
using namespace afl::util;
using namespace os::v2x::protocol::om::db;
using namespace os::v2x::protocol::om::common;
//class DeviceStatusMsgDB;
class DBUtils
{
    public:
    DBUtils() = default;
    virtual ~DBUtils() = default;// 这里使用 default 表示使用默认析构函数  ;
    static void getDBInfo(std::pair<std::string, TABLE_TYPE> tableInfo, std::vector<MsgDeviceStatus> &statusList, std::string interface="", std::string DbFilePath = "/home/airos/protocol/device-status.db");
    static void updateDBInfo(std::pair<std::string, TABLE_TYPE> tableInfo, MsgDeviceStatus msgDeviceStatus, std::string interface="", std::string DbFilePath = "/home/airos/protocol/device-status.db");
    static void initDBInfo(std::pair<std::string, TABLE_TYPE> tableInfo, MsgDeviceStatus msgDeviceStatus, std::string interface="", std::string DbFilePath = "/home/airos/protocol/device-status.db");
    static void delDBInfo(std::pair<std::string, TABLE_TYPE> tableInfo, MsgDeviceStatus msgDeviceStatus, std::string interface="", std::string DbFilePath = "/home/airos/protocol/device-status.db");
    static std::string getLastPartOfPath(const std::string &path);
    static std::string extractVersion(const std::string &input);
    static std::string trim(const std::string &str);
    static std::string getGatewayAddress();
    static NetworkInfo getNetworkInfo(const std::string &interface_name);

    static bool sshExec(SshInfo &sshInfo);
    static bool executeCommand(ssh_session session, const std::string &command, std::string &execRet, bool flagReboot = false);


    // 静态初始化函数
    static void initializeDB(const std::string &DbFilePath = "/home/airos/protocol/device-status.db");
    // 判断是否已初始化
    static bool isDBInitialized();
    // 静态属性
    static std::unique_ptr<DeviceStatusMsgDB> m_DeviceStatusMsgDBPtr;
    static void setEnableDebugPrint(bool enableDebugPrint);
public:
    static bool enableDebugPrint;
};
NAMESPACE_ENDED_OM_COMPONENT_COMMON
#endif
