/*********************************************************************************
* @file		sqlite_device_status.h.h
* @brief		sqlite_device_status.h belongs to CICTCI
* @details
* @author		alfred
* @email       zhangenwei64@gmail.com
* @date		24-6-2
* @copyright	Copyright (c) 2024 Mec-Airos Division.
* @verbatim
*
*  Change History:
*  Date      Author    Version  ChangeId           Description
*  ------------------------------------------------------------------------------
*  24-5-29 alfred       1.0       ————             Create this file
*
* @endverbatim
********************************************************************************/
#ifndef AIROS2_0_SQLITE_DEVICE_STATUS_H
#define AIROS2_0_SQLITE_DEVICE_STATUS_H
#include "namespace.h"
#include "configer_common.h"
#include "configer_topic_om_mec.h"
#include "data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_DEVICE_STATUS
using namespace os::v2x::protocol::om::db;
using namespace std;
using namespace os::v2x::protocol::om::common;
class DeviceStatusMsgDB
{
public:
    DeviceStatusMsgDB() = default;
    DeviceStatusMsgDB(std::string databasePath = "/home/airos/protocol/device-status.db", bool enableDebugPrintTemp = false);
    virtual ~DeviceStatusMsgDB();
    void setDatabaseProperties();
    void executeSQL(const std::string& sql);
    std::string getTableUpdateSql(std::pair<std::string, TABLE_TYPE> tableInfo, TABLE_OPERA_TYPE operType, const MsgDeviceStatus& status);
    bool insertOrUpdateDeviceStatus(std::pair<std::string, TABLE_TYPE> tableInfo, const MsgDeviceStatus& status, std::string interface="");
    bool getAllDeviceStatus(std::pair<std::string, TABLE_TYPE> tableInfo,std::vector<MsgDeviceStatus>& statusList, std::string interface="");
    bool CreatTable(std::pair<std::string, TABLE_TYPE> tableInfo, std::string interface = ""); // 打开数据库中的表，如果存在。不做任何处理
    bool ClearTable(std::pair<std::string, TABLE_TYPE> tableInfo, std::string interface = "");
    void CloseDB();    // 关闭数据库
    bool insertOrUpdateDeviceStatusSomeField(std::pair<std::string, TABLE_TYPE> tableInfo, const MsgDeviceStatus& status, std::string interface="");
    std::string getTableUpdateSqlSomeField(std::pair<std::string, TABLE_TYPE> tableInfo, TABLE_OPERA_TYPE operType, const MsgDeviceStatus& status);
    bool deleteTableSomeLine(std::pair<std::string, TABLE_TYPE> tableInfo, const MsgDeviceStatus& status, std::string interface="");
private:
    bool OpenDB(const string &dbPath);        // 打开数据库，如果不存在, 则创建数据库
private:
    sqlite3 *sqlite_;     // 连接数据库的指针
    string datebaseName_; // 数据库中表的名字
    bool enableDebugPrint;
};
NAMESPACE_ENDED_OM_COMPONENT_DEVICE_STATUS
#endif //AIROS2_0_SQLITE_DEVICE_STATUS_H
