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

#include "sqlite_device_status.h"
NAMESPACE_START_OM_COMPONENT_DEVICE_STATUS
DeviceStatusMsgDB::DeviceStatusMsgDB(std::string databasePath, bool enableDebugPrintTemp) : sqlite_(nullptr),
                                                                 datebaseName_(databasePath), enableDebugPrint(enableDebugPrintTemp)
{
    // 打开数据库, 当数据库不存在时，自动创建。
    OpenDB(datebaseName_);
    setDatabaseProperties();
    enableDebugPrint = enableDebugPrintTemp;
}
DeviceStatusMsgDB::~DeviceStatusMsgDB()
{
    CloseDB();
}
void DeviceStatusMsgDB::setDatabaseProperties()
{
    std::string sql;
    int milliseconds = 2000;

    sql = "PRAGMA busy_timeout = " + std::to_string(milliseconds) + ";";   
    executeSQL(sql);

    sql = "PRAGMA journal_mode = WAL;";
    executeSQL(sql);
}
void DeviceStatusMsgDB::executeSQL(const std::string& sql)
{
    char* errorMessage = nullptr;
    int result = sqlite3_exec(sqlite_, sql.c_str(), nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        OM_DS_ERROR_PRINT << "[Error] SQL execution failed: " << errorMessage;
        sqlite3_free(errorMessage);
    }
}
bool DeviceStatusMsgDB::OpenDB(const string &dbPath)
{
    int result = sqlite3_open(dbPath.c_str(), &sqlite_);
    if (result == SQLITE_OK)
    {
//        OM_DS_SUCCESS_PRINT << "[Sucess]Open datebase successful,the datebase path " << dbPath.c_str();
        return true;
    }
    else
    {
        OM_DS_ERROR_PRINT << "[error]Open datebase fail,the datebase path " << dbPath.c_str();
        return false;
    }
}
void DeviceStatusMsgDB::CloseDB()
{
    if (sqlite_)
    {
        sqlite3_close(sqlite_);
        sqlite_ = nullptr;
    }
}
bool DeviceStatusMsgDB::CreatTable(std::pair<std::string, TABLE_TYPE> tableInfo, std::string interface)
{
    std::string sql;
    switch(tableInfo.second)
    {
        case TABLE_TYEP_SENSOR_DEV_STATUS:  //感知设备状态表
            sql = "CREATE TABLE IF NOT EXISTS [" + tableInfo.first + "] ("
                                                                      "\'TIME_STAMP\' INTEGER, "
                                                                      "\'DEVICE_TYPE\' INTEGER, "
                                                                      "\'DEVICE_ID\' TEXT, "
                                                                      "\'DEVICE_STATUS\' INTEGER, "
                                                                      "\'DEVICE_ACTIVE\' INTEGER, "
                                                                      "\'MEC_ESN\' TEXT, "
                                                                      "\'DEVICE_IP\' TEXT, "
                                                                      "\'LONGITUDE\' REAL, "
                                                                      "\'LATITUDE\' REAL, "
                                                                      "\'ALTITUDE\' REAL, "
                                                                      "\'ALARM_TYPE\' INTEGER, "
                                                                      "\'ADDITION\' TEXT "
                                                                      ")";
            break;
        case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:  //感知设备授时表
            sql = "CREATE TABLE IF NOT EXISTS [" + tableInfo.first + "] ("
                                                                      "\'TIME_STAMP\' INTEGER, "
                                                                      "\'DEVICE_TYPE\' INTEGER, "
                                                                      "\'DEVICE_ID\' TEXT, "
                                                                      "\'DEVICE_STATUS\' INTEGER, "
                                                                      "\'DEV_TIME\' INTEGER, "
                                                                      "\'LAST_TIME\' INTEGER, "
                                                                      "\'TIME_DIFF\' INTEGER "
                                                                      ")";
            break;
        case TABLE_TYEP_MEC_DEV_STATUS:  //MEC设备状态表
            sql = "CREATE TABLE IF NOT EXISTS [" + tableInfo.first + "] ("
                                                                        "\'TIME_STAMP\' INTEGER, "
                                                                        "\'DEVICE_ID\' TEXT, "
                                                                        "\'DEVICE_TYPE\' INTEGER, "
                                                                        "\'DEVICE_STATUS\' INTEGER, "
                                                                        "\'DEVICE_ACTIVE\' INTEGER, "
                                                                        "\'SOFTWARE_VERSION\' TEXT, "
                                                                        "\'LAST_TIME\' INTEGER, "
                                                                        "\'TIME_NODE\' INTEGER, "
                                                                        "\'TIME_DIFF\' INTEGER, "
                                                                        "\'POINT_NO\' TEXT, "
                                                                        "\'POINT_NAME\' TEXT, "
                                                                        "\'LON\' REAL, "
                                                                        "\'LAT\' REAL, "
                                                                        "\'DEVICE_IP\' TEXT, "
                                                                        "\'NET_MASK\' TEXT, "
                                                                        "\'GATEWAY\' TEXT, "
                                                                        "\'CPU_LOAD\' TEXT, "
                                                                        "\'CPU_TEMP\' REAL, "
                                                                        "\'CPU_UTI\' TEXT, "
                                                                        "\'GPU_LOAD\' REAL, "
                                                                        "\'GPU_SMEM\' REAL, "
                                                                        "\'GPU_TEMP\' REAL, "
                                                                        "\'GPU_UTI\' TEXT, "
                                                                        "\'MEM_TOTAL\' REAL, "
                                                                        "\'MEM_USED\' REAL, "
                                                                        "\'MEM_FREE\' REAL, "
                                                                        "\'DISK_TOTAL\' REAL, "
                                                                        "\'DISK_USED\' REAL, "
                                                                        "\'DISK_FREE\' REAL, "
                                                                        "\'DISK_TPS\' INTEGER, "
                                                                        "\'DISK_WRITE\' REAL, "
                                                                        "\'DISK_READ\' REAL, "
                                                                         "\'NET_RX\' INTEGER, "
                                                                         "\'NET_TX\' INTEGER, "
                                                                         "\'NET_RX_BYTE\' REAL, "
                                                                         "\'NET_TX_BYTE\' REAL "
                                                                        ")";
            break;
        case TABLE_TYEP_MEC_DEV_ALARM:  //MEC设备告警表
            sql = "CREATE TABLE IF NOT EXISTS [" + tableInfo.first + "] ("
                                                                      "\'TIME_STAMP\' INTEGER, "
                                                                      "\'ALARM_TYPE\' INTEGER, "
                                                                      "\'ALARM_LEVEL\' INTEGER, "
                                                                      "\'ALARM_STATUS\' INTEGER, "
                                                                      "\'ALARM_RAISED_TIME\' INTEGER, "
                                                                      "\'ALARM_CHANGED_TIME\' INTEGER, "
                                                                      "\'ADDITION\' TEXT "
                                                                      ")";
            break;
        case TABLE_TYEP_RADAR_DEV_ALARM:  //雷达设备告警表
            sql = "CREATE TABLE IF NOT EXISTS [" + tableInfo.first + "] ("
                                                                     "\'TIME_STAMP\' INTEGER, "
                                                                     "\'DEVICE_NO_ALARM_TYPE\' TEXT, "
                                                                     "\'ALARM_TYPE\' INTEGER, "
                                                                     "\'ALARM_LEVEL\' INTEGER, "
                                                                     "\'ALARM_STATUS\' INTEGER, "
                                                                     "\'ALARM_RAISED_TIME\' INTEGER, "
                                                                     "\'ALARM_CHANGED_TIME\' INTEGER, "
                                                                     "\'ADDITION\' TEXT "
                                                                     ")";
            break;
        case TABLE_TYEP_CAMERA_DEV_ALARM:  //摄像头设备告警表
            sql = "CREATE TABLE IF NOT EXISTS [" + tableInfo.first + "] ("
                                                                     "\'TIME_STAMP\' INTEGER, "
                                                                     "\'DEVICE_NO_ALARM_TYPE\' TEXT, "
                                                                     "\'ALARM_TYPE\' INTEGER, "
                                                                     "\'ALARM_LEVEL\' INTEGER, "
                                                                     "\'ALARM_STATUS\' INTEGER, "
                                                                     "\'ALARM_RAISED_TIME\' INTEGER, "
                                                                     "\'ALARM_CHANGED_TIME\' INTEGER, "
                                                                     "\'ADDITION\' TEXT "
                                                                     ")";
            break;
        default:
            break;
    }

    if(sql.empty())
    {
        return false;
    }
    if (enableDebugPrint)
    {
        OM_DS_DEBUG_PRINT << "[sql]" <<  sql.c_str();
    }

    char *sErrMsg = nullptr;
    if (sqlite3_exec(sqlite_, sql.c_str(), NULL, NULL, &sErrMsg) != SQLITE_OK)
    {
        OM_DS_ERROR_PRINT << interface << "[error]Creat datebase table " << tableInfo.first.c_str() << " fail, " << sErrMsg;
        return false;
    }
    sqlite3_free(sErrMsg);
    return true;
}

std::string DeviceStatusMsgDB::getTableUpdateSql(std::pair<std::string, TABLE_TYPE> tableInfo, TABLE_OPERA_TYPE operType, const MsgDeviceStatus& status)
{
    std::string sql;
    switch(operType)
    {
        case  TABLE_OPERA_TYPE_UPDATE:  //更新操作
            switch(tableInfo.second)
            {
                case TABLE_TYEP_SENSOR_DEV_STATUS:  //设备状态表
                    sql = "UPDATE [" + tableInfo.first + "] SET TIME_STAMP = ?, DEVICE_TYPE = ?, DEVICE_STATUS = ?, DEVICE_ACTIVE = ?,  MEC_ESN = ?, DEVICE_IP = ?, LONGITUDE = ?, LATITUDE = ?, ALTITUDE = ?,"
                                                          "  ALARM_TYPE = ?, ADDITION = ?"
                                                          " WHERE DEVICE_ID = ?";
                    break;
                case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:  //设备授时表
                    sql = "UPDATE [" + tableInfo.first + "] SET TIME_STAMP = ?, DEVICE_TYPE = ?, DEVICE_STATUS = ?, DEV_TIME = ?, LAST_TIME = ?, TIME_DIFF = ? "
                                                          " WHERE DEVICE_ID = ?";
                    break;
                case TABLE_TYEP_MEC_DEV_STATUS:  //MEC状态表
                    sql = "UPDATE [" + tableInfo.first + "] SET TIME_STAMP = ?, DEVICE_TYPE = ?, DEVICE_STATUS = ?, DEVICE_ACTIVE = ?, SOFTWARE_VERSION = ?, "
                                                          "LAST_TIME = ?, TIME_NODE = ?, TIME_DIFF = ?, POINT_NO = ?, POINT_NAME = ?, LON = ?, LAT = ?, DEVICE_IP = ?, "
                                                          "NET_MASK = ?, GATEWAY = ?, CPU_LOAD = ?, CPU_TEMP = ?, CPU_UTI = ?, GPU_LOAD = ?, GPU_SMEM = ?, GPU_TEMP = ?, "
                                                          "GPU_UTI = ?, MEM_TOTAL = ?, MEM_USED = ?, MEM_FREE = ?, DISK_TOTAL = ?, DISK_USED = ?, DISK_FREE = ?, DISK_TPS = ?, "
                                                          "DISK_WRITE = ?, DISK_READ = ?, NET_RX = ?, NET_TX = ?, NET_RX_BYTE = ?, NET_TX_BYTE = ? WHERE DEVICE_ID = ?";
                    break;
                case TABLE_TYEP_MEC_DEV_ALARM:  //MEC告警表
                    sql = "UPDATE [" + tableInfo.first + "] SET TIME_STAMP = ?, ALARM_LEVEL = ?, ALARM_STATUS = ?, ALARM_RAISED_TIME = ?, ALARM_CHANGED_TIME = ?, ADDITION = ? WHERE ALARM_TYPE = ?";
                    break;
                case TABLE_TYEP_RADAR_DEV_ALARM:  //雷达告警表
                    sql = "UPDATE [" + tableInfo.first + "] SET TIME_STAMP = ?, ALARM_TYPE = ?, ALARM_LEVEL = ?, ALARM_STATUS = ?, ALARM_RAISED_TIME = ?, ALARM_CHANGED_TIME = ?, ADDITION = ? WHERE DEVICE_NO_ALARM_TYPE = ?";
                    break;
                case TABLE_TYEP_CAMERA_DEV_ALARM:  //相机告警表
                    sql = "UPDATE [" + tableInfo.first + "] SET TIME_STAMP = ?, ALARM_TYPE = ?,  ALARM_LEVEL = ?, ALARM_STATUS = ?, ALARM_RAISED_TIME = ?, ALARM_CHANGED_TIME = ?, ADDITION = ? WHERE DEVICE_NO_ALARM_TYPE = ?";
                    break;
                default:
                    break;
            }
            break;
        case TABLE_OPERA_TYPE_INSERT:  //插入
            switch(tableInfo.second)
            {
                case TABLE_TYEP_SENSOR_DEV_STATUS:  //设备状态表
                    sql = "INSERT INTO [" + tableInfo.first + "] (TIME_STAMP, DEVICE_TYPE, DEVICE_ID, DEVICE_STATUS, DEVICE_ACTIVE, MEC_ESN, DEVICE_IP, LONGITUDE, LATITUDE, ALTITUDE, "
                                                               " ALARM_TYPE, ADDITION"
                                                               ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
                    break;
                case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:  //设备授时表
                    sql = "INSERT INTO [" + tableInfo.first + "] (TIME_STAMP, DEVICE_TYPE, DEVICE_ID, DEVICE_STATUS, DEV_TIME, LAST_TIME, TIME_DIFF"
                                                               ") VALUES (?, ?, ?, ?, ?, ?, ?)";
                    break;
                case TABLE_TYEP_MEC_DEV_STATUS:  //MEC状态表
                    sql = "INSERT INTO [" + tableInfo.first + "] (TIME_STAMP, DEVICE_ID, DEVICE_TYPE,  DEVICE_STATUS, DEVICE_ACTIVE, SOFTWARE_VERSION, "
                                                               "LAST_TIME, TIME_NODE, TIME_DIFF, POINT_NO, POINT_NAME, LON, LAT, DEVICE_IP, NET_MASK, GATEWAY, "
                                                               "CPU_LOAD, CPU_TEMP, CPU_UTI, GPU_LOAD, GPU_SMEM, GPU_TEMP, GPU_UTI, "
                                                               "MEM_TOTAL, MEM_USED, MEM_FREE, DISK_TOTAL, DISK_USED, DISK_FREE, DISK_TPS, DISK_WRITE, DISK_READ, NET_RX, NET_TX, NET_RX_BYTE, NET_TX_BYTE) "
                                                               "VALUES ("
                                                               "?, ?, ?, ?, ?, ?, "
                                                               "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
                                                               "?, ?, ?, ?, ?, ?, ?, "
                                                               "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
                    break;
                case TABLE_TYEP_MEC_DEV_ALARM:  //MEC告警表
                    sql = "INSERT INTO [" + tableInfo.first + "] (TIME_STAMP, ALARM_TYPE, ALARM_LEVEL, ALARM_STATUS, ALARM_RAISED_TIME, ALARM_CHANGED_TIME, ADDITION) "
                                                               "VALUES (?, ?, ?, ?, ?, ?, ?)";
                    break;
                case TABLE_TYEP_RADAR_DEV_ALARM:  //雷达告警表
                    sql = "INSERT INTO [" + tableInfo.first + "] (TIME_STAMP, DEVICE_NO_ALARM_TYPE, ALARM_TYPE, ALARM_LEVEL, ALARM_STATUS, ALARM_RAISED_TIME, ALARM_CHANGED_TIME, ADDITION) "
                                                              "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
                    break;
                case TABLE_TYEP_CAMERA_DEV_ALARM:  //相机告警表
                    sql = "INSERT INTO [" + tableInfo.first + "] (TIME_STAMP, DEVICE_NO_ALARM_TYPE, ALARM_TYPE, ALARM_LEVEL, ALARM_STATUS, ALARM_RAISED_TIME, ALARM_CHANGED_TIME, ADDITION) "
                                                              "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
                    break;
                default:
                    break;
            }
            break;

        default:

            break;

    }
    return sql;
};

bool DeviceStatusMsgDB::insertOrUpdateDeviceStatus(std::pair<std::string, TABLE_TYPE> tableInfo, const MsgDeviceStatus& status, std::string interface)
{
    sqlite3_stmt* stmt;
    int count = 0;
    std::string sql;
    if(tableInfo.second == TABLE_TYEP_MEC_DEV_ALARM)
    {
        sql = "SELECT COUNT(*) FROM [" + tableInfo.first + "] WHERE ALARM_TYPE = ?";
    }
    else
    {
        sql = "SELECT COUNT(*) FROM [" + tableInfo.first + "] WHERE DEVICE_ID = ?";
    }

    int rc = sqlite3_prepare_v2(sqlite_, sql.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            count = sqlite3_column_int(stmt, 0);
        }
        else
        {
            OM_DS_ERROR_PRINT << interface << "[error]Check exists fail";
            rc = -1;
            return false;
        }
        //        sqlite3_finalize(stmt);
    }
    else
    {
        OM_DS_ERROR_PRINT << interface << "[error]Check exists fail " << rc;
        rc = -1;
        return false;
    }
    if (enableDebugPrint)
    {
        OM_DS_DEBUG_PRINT << interface << "[count]" << count << "[table]" << tableInfo.first;
    }
    if (count > 0)
    {
        std::string sql = getTableUpdateSql(tableInfo, TABLE_OPERA_TYPE_UPDATE, status);
        int rc = sqlite3_prepare_v2(sqlite_, sql.c_str(), -1, &stmt, NULL);
        if (rc == SQLITE_OK)
        {
            switch (tableInfo.second)
            {
                case TABLE_TYEP_SENSOR_DEV_STATUS:
                    {
                        int index = 1;
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);
                        sqlite3_bind_int(stmt, index++, status.deviceType);
                        sqlite3_bind_int(stmt, index++, status.deviceStatus);
                        sqlite3_bind_int(stmt, index++, status.active);
                        sqlite3_bind_text(stmt, index++, status.mecEsn.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_bind_text(stmt, index++, status.deviceIp.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_bind_double(stmt, index++, status.longitude);
                        sqlite3_bind_double(stmt, index++, status.latitude);
                        sqlite3_bind_double(stmt, index++, status.altitude);

                        sqlite3_bind_int(stmt, index++, status.alarmType);
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT);

                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
                    }
                    break;
                case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:
                    {
                        int index = 1;
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);
                        sqlite3_bind_int(stmt, index++, status.deviceType);
                        sqlite3_bind_int(stmt, index++, status.deviceStatus);
                        sqlite3_bind_int64(stmt, index++, status.devTime);
                        sqlite3_bind_int64(stmt, index++, status.lastTime);
                        sqlite3_bind_int64(stmt, index++, status.timeDiff);
                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
                    }
                    break;
                case TABLE_TYEP_MEC_DEV_STATUS:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_int(stmt, index++, status.deviceType);           // DEVICE_TYPE
                        sqlite3_bind_int(stmt, index++, status.deviceStatus);         // DEVICE_STATUS
                        sqlite3_bind_int(stmt, index++, status.active);               // DEVICE_ACTIVE
                        sqlite3_bind_text(stmt, index++, status.softwareVersion.c_str(), -1, SQLITE_TRANSIENT); // SOFTWARE_VERSION
                        sqlite3_bind_int64(stmt, index++, status.lastTime);           // LAST_TIME
                        sqlite3_bind_int64(stmt, index++, status.timeNode);           // TIME_NODE
                        sqlite3_bind_int(stmt, index++, status.timeDiff);       // TIME_DIFFERENCE
                        sqlite3_bind_text(stmt, index++, status.pointNo.c_str(), -1, SQLITE_TRANSIENT);              // POINT_NO
                        sqlite3_bind_text(stmt, index++, status.pointName.c_str(), -1, SQLITE_TRANSIENT); // POINT_NAME
                        sqlite3_bind_double(stmt, index++, status.longitude);         // LON
                        sqlite3_bind_double(stmt, index++, status.latitude);          // LAT
                        sqlite3_bind_text(stmt, index++, status.deviceIp.c_str(), -1, SQLITE_TRANSIENT); // IP
                        sqlite3_bind_text(stmt, index++, status.netMask.c_str(), -1, SQLITE_TRANSIENT);   // NET_MASK
                        sqlite3_bind_text(stmt, index++, status.gateway.c_str(), -1, SQLITE_TRANSIENT);     // GATEWAY
                        sqlite3_bind_text(stmt, index++, status.cpuLoad.c_str(), -1, SQLITE_TRANSIENT);           // CPU_LOAD
                        sqlite3_bind_double(stmt, index++, status.cpuTemp);              // CPU_TEMP
                        sqlite3_bind_text(stmt, index++, status.cpuUti.c_str(), -1, SQLITE_TRANSIENT);          // CPU_UTI
                        sqlite3_bind_double(stmt, index++, status.gpuLoad);              // GPU_LOAD
                        sqlite3_bind_double(stmt, index++, status.gpuSmem);              // GPU_SMEM
                        sqlite3_bind_double(stmt, index++, status.gpuTemp);              // GPU_TEMP
                        sqlite3_bind_text(stmt, index++, status.gpuUti.c_str(), -1, SQLITE_TRANSIENT);               // GPU_UTI
                        sqlite3_bind_double(stmt, index++, status.memTotal);           // MEM_TOTAL
                        sqlite3_bind_double(stmt, index++, status.memUsed);            // MEM_USED
                        sqlite3_bind_double(stmt, index++, status.memFree);            // MEM_FREE
                        sqlite3_bind_double(stmt, index++, status.diskTotal);          // DISK_TOTAL
                        sqlite3_bind_double(stmt, index++, status.diskUsed);           // DISK_USED
                        sqlite3_bind_double(stmt, index++, status.diskFree);           // DISK_FREE
                        sqlite3_bind_double(stmt, index++, status.diskTps);              // DISK_TPS
                        sqlite3_bind_double(stmt, index++, status.diskWrite);          // DISK_WRITE
                        sqlite3_bind_double(stmt, index++, status.diskRead);           // DISK_READ
                        
                        sqlite3_bind_int64(stmt, index++, status.netRx);           // netRx
                        sqlite3_bind_int64(stmt, index++, status.netTx);           // netTx
                        sqlite3_bind_double(stmt, index++, status.netRxByte);          // netRxByte
                        sqlite3_bind_double(stmt, index++, status.netTxByte);           // netTxByte
                        
                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT); // DEVICE_ID
                    }
                    break;
                    case TABLE_TYEP_MEC_DEV_ALARM:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_int(stmt, index++, status.alarmLevel);           // ALARM_LEVEL
                        sqlite3_bind_int(stmt, index++, status.alarmStatus);          // ALARM_STATUS
                        sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);    // ALARM_RAISED_TIME
                        sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);   // ALARM_CHANGED_TIME
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT); // ALARM_TYPE
                        sqlite3_bind_int(stmt, index++, status.alarmType);   // ALARM_TYPE
                    }
                    break;
                    case TABLE_TYEP_RADAR_DEV_ALARM:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_int(stmt, index++, status.alarmType);   // ALARM_TYPE
                        sqlite3_bind_int(stmt, index++, status.alarmLevel);           // ALARM_LEVEL
                        sqlite3_bind_int(stmt, index++, status.alarmStatus);          // ALARM_STATUS
                        sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);    // ALARM_RAISED_TIME
                        sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);   // ALARM_CHANGED_TIME
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT); // ALARM_TYPE
                        sqlite3_bind_text(stmt, index++, status.deviceNoAlarmType.c_str(), -1, SQLITE_TRANSIENT);   // DEVICE_NO_ALARM_TYPE
                    }
                    break;
                    case TABLE_TYEP_CAMERA_DEV_ALARM:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_int(stmt, index++, status.alarmType);   // ALARM_TYPE
                        sqlite3_bind_int(stmt, index++, status.alarmLevel);           // ALARM_LEVEL
                        sqlite3_bind_int(stmt, index++, status.alarmStatus);          // ALARM_STATUS
                        sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);    // ALARM_RAISED_TIME
                        sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);   // ALARM_CHANGED_TIME
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT); // ALARM_TYPE
                        sqlite3_bind_text(stmt, index++, status.deviceNoAlarmType.c_str(), -1, SQLITE_TRANSIENT);   // DEVICE_NO_ALARM_TYPE
                    }
                    break;
                default:
                    break;
            }

            if(!stmt)
            {
                OM_DS_ERROR_PRINT << interface << "[error]stmt is null!";
                return false;
            }
            char* expandedSql = sqlite3_expanded_sql(stmt);
//            OM_DS_WARN_PRINT << "[sql]" << expandedSql;
            sqlite3_free(expandedSql);
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                OM_DS_ERROR_PRINT << interface << "[error]: Update fail " << rc;
                rc = -1;
                return false;
            }
            if (enableDebugPrint)
            {
                OM_DS_SUCCESS_PRINT << interface << SUCESS_COLOR_STR << "[sucess]update success!" << SUCESS_COLOR_END;
            }
            sqlite3_finalize(stmt);
        }
        else
        {
            OM_DS_ERROR_PRINT << interface << "[error]: Update fail " << rc;
            rc = -1;
            return false;
        }
    }
    else
    {
        std::string sql = getTableUpdateSql(tableInfo, TABLE_OPERA_TYPE_INSERT, status);
//	    OM_DS_WARN_PRINT << "[sql]" << sql;
        int rc = sqlite3_prepare_v2(sqlite_, sql.c_str(), -1, &stmt, NULL);
        if (rc == SQLITE_OK)
        {
            switch (tableInfo.second)
            {
                case TABLE_TYEP_SENSOR_DEV_STATUS:
                    {
                        int index = 1;
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);
                        sqlite3_bind_int(stmt, index++, status.deviceType);
                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_bind_int(stmt, index++, status.deviceStatus);
                        sqlite3_bind_int(stmt, index++, status.active);
                        sqlite3_bind_text(stmt, index++, status.mecEsn.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_bind_text(stmt, index++, status.deviceIp.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_bind_double(stmt, index++, status.longitude);
                        sqlite3_bind_double(stmt, index++, status.latitude);
                        sqlite3_bind_double(stmt, index++, status.altitude);
                        sqlite3_bind_int(stmt, index++, status.alarmType);
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT);
                    }
                    break;
                case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:
                    {
                        int index = 1;
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);
                        sqlite3_bind_int(stmt, index++, status.deviceType);
                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_bind_int(stmt, index++, status.deviceStatus);
                        sqlite3_bind_int64(stmt, index++, status.devTime);
                        sqlite3_bind_int64(stmt, index++, status.lastTime);
                        sqlite3_bind_int64(stmt, index++, status.timeDiff);
                    }
                    break;
                case TABLE_TYEP_MEC_DEV_STATUS:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT); // DEVICE_ID
                        sqlite3_bind_int(stmt, index++, status.deviceType);           // DEVICE_TYPE
                        sqlite3_bind_int(stmt, index++, status.deviceStatus);         // DEVICE_STATUS
                        sqlite3_bind_int(stmt, index++, status.active);               // DEVICE_ACTIVE
                        sqlite3_bind_text(stmt, index++, status.softwareVersion.c_str(), -1, SQLITE_TRANSIENT); // SOFTWARE_VERSION
                        sqlite3_bind_int64(stmt, index++, status.lastTime);           // LAST_TIME
                        sqlite3_bind_int64(stmt, index++, status.timeNode);           // TIME_NODE
                        sqlite3_bind_int(stmt, index++, status.timeDiff);       // TIME_DIFFERENCE
                        sqlite3_bind_text(stmt, index++, status.pointNo.c_str(), -1, SQLITE_TRANSIENT);              // POINT_NO
                        sqlite3_bind_text(stmt, index++, status.pointName.c_str(), -1, SQLITE_TRANSIENT); // POINT_NAME
                        sqlite3_bind_double(stmt, index++, status.longitude);         // LON
                        sqlite3_bind_double(stmt, index++, status.latitude);          // LAT
                        sqlite3_bind_text(stmt, index++, status.deviceIp.c_str(), -1, SQLITE_TRANSIENT); // IP
                        sqlite3_bind_text(stmt, index++, status.netMask.c_str(), -1, SQLITE_TRANSIENT);   // NET_MASK
                        sqlite3_bind_text(stmt, index++, status.gateway.c_str(), -1, SQLITE_TRANSIENT);     // GATEWAY
                        sqlite3_bind_text(stmt, index++, status.cpuLoad.c_str(), -1, SQLITE_TRANSIENT);            // CPU_LOAD
                        sqlite3_bind_double(stmt, index++, status.cpuTemp);              // CPU_TEMP
                        sqlite3_bind_text(stmt, index++, status.cpuUti.c_str(), -1, SQLITE_TRANSIENT);              // CPU_UTI
                        sqlite3_bind_double(stmt, index++, status.gpuLoad);              // GPU_LOAD
                        sqlite3_bind_double(stmt, index++, status.gpuSmem);              // GPU_SMEM
                        sqlite3_bind_double(stmt, index++, status.gpuTemp);              // GPU_TEMP
                        sqlite3_bind_text(stmt, index++, status.gpuUti.c_str(), -1, SQLITE_TRANSIENT);               // GPU_UTI
                        sqlite3_bind_double(stmt, index++, status.memTotal);           // MEM_TOTAL
                        sqlite3_bind_double(stmt, index++, status.memUsed);            // MEM_USED
                        sqlite3_bind_double(stmt, index++, status.memFree);            // MEM_FREE
                        sqlite3_bind_double(stmt, index++, status.diskTotal);          // DISK_TOTAL
                        sqlite3_bind_double(stmt, index++, status.diskUsed);           // DISK_USED
                        sqlite3_bind_double(stmt, index++, status.diskFree);           // DISK_FREE
                        sqlite3_bind_double(stmt, index++, status.diskTps);              // DISK_TPS
                        sqlite3_bind_double(stmt, index++, status.diskWrite);          // DISK_WRITE
                        sqlite3_bind_double(stmt, index++, status.diskRead);           // DISK_READ
                        sqlite3_bind_int64(stmt, index++, status.netRx);           // netRx
                        sqlite3_bind_int64(stmt, index++, status.netTx);           // netTx
                        sqlite3_bind_double(stmt, index++, status.netRxByte);          // netRxByte
                        sqlite3_bind_double(stmt, index++, status.netTxByte);           // netTxByte
                    }
                    break;
                    case TABLE_TYEP_MEC_DEV_ALARM:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_int(stmt, index++, status.alarmType);           //  ALARM_TYPE
                        sqlite3_bind_int(stmt, index++, status.alarmLevel);           // ALARM_LEVEL
                        sqlite3_bind_int(stmt, index++, status.alarmStatus);          // ALARM_STATUS
                        sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);    // ALARM_RAISED_TIME
                        sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);   // ALARM_CHANGED_TIME
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT); //
                    }
                    break;
                    case TABLE_TYEP_RADAR_DEV_ALARM:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_text(stmt, index++, status.deviceNoAlarmType.c_str(), -1, SQLITE_TRANSIENT); //DEVICE_NO_ALARM_TYPE
                        sqlite3_bind_int(stmt, index++, status.alarmType);           //  ALARM_TYPE
                        sqlite3_bind_int(stmt, index++, status.alarmLevel);           // ALARM_LEVEL
                        sqlite3_bind_int(stmt, index++, status.alarmStatus);          // ALARM_STATUS
                        sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);    // ALARM_RAISED_TIME
                        sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);   // ALARM_CHANGED_TIME
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT); //
                    }
                    break;
                    case TABLE_TYEP_CAMERA_DEV_ALARM:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_text(stmt, index++, status.deviceNoAlarmType.c_str(), -1, SQLITE_TRANSIENT); //DEVICE_NO_ALARM_TYPE
                        sqlite3_bind_int(stmt, index++, status.alarmType);           //  ALARM_TYPE
                        sqlite3_bind_int(stmt, index++, status.alarmLevel);           // ALARM_LEVEL
                        sqlite3_bind_int(stmt, index++, status.alarmStatus);          // ALARM_STATUS
                        sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);    // ALARM_RAISED_TIME
                        sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);   // ALARM_CHANGED_TIME
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT); //
                    }
                    break;
                default:
                    break;
            }
            if(!stmt)
            {
                OM_DS_ERROR_PRINT << interface << "[error]stmt is null!";
                return false;
            }
            char* expandedSql = sqlite3_expanded_sql(stmt);
            if (enableDebugPrint)
            {
                OM_DS_DEBUG_PRINT << interface << "[sql]" << expandedSql;
            }
            if (!expandedSql)
            {
                OM_DS_ERROR_PRINT << interface << "[error]expandedSql is null!";
            }
            sqlite3_free(expandedSql);
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                OM_DS_DEBUG_PRINT << interface  << "[error]: Update fail " << rc;
                rc = -1;
                return false;
            }
            if (enableDebugPrint)
            {
                OM_DS_SUCCESS_PRINT << interface  << "[sucess]insert success!";
            }
            sqlite3_finalize(stmt);
        }
        else
        {
	        OM_DS_ERROR_PRINT << interface  << "[error]: Update fail " << rc;
	        rc = -1;
	        return false;
        }
    }

    return true;
}

bool DeviceStatusMsgDB::getAllDeviceStatus(std::pair<std::string, TABLE_TYPE> tableInfo, std::vector<MsgDeviceStatus>& statusList, std::string interface)
{
    std::vector<std::string> kColumnNames;
    switch (tableInfo.second)
    {
        case TABLE_TYEP_SENSOR_DEV_STATUS:
            kColumnNames = {"TIME_STAMP", "DEVICE_TYPE", "DEVICE_ID", "DEVICE_STATUS", "DEVICE_ACTIVE", "MEC_ESN", "DEVICE_IP", "LONGITUDE", "LATITUDE", "ALTITUDE",
                            "ALARM_TYPE", "ADDITION"};
            break;
        case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:
            kColumnNames = {"TIME_STAMP", "DEVICE_TYPE", "DEVICE_ID", "DEVICE_STATUS", "DEV_TIME", "LAST_TIME", "TIME_DIFF"};
            break;
        case TABLE_TYEP_MEC_DEV_STATUS:
            kColumnNames = {"TIME_STAMP",  "DEVICE_ID", "DEVICE_TYPE", "DEVICE_STATUS", "DEVICE_ACTIVE", "SOFTWARE_VERSION",
                            "LAST_TIME", "TIME_NODE", "TIME_DIFF", "POINT_NO", "POINT_NAME",
                            "LON", "LAT", "DEVICE_IP", "NET_MASK", "GATEWAY", "CPU_LOAD",
                            "CPU_TEMP", "CPU_UTI", "GPU_LOAD", "GPU_SMEM", "GPU_TEMP",
                            "GPU_UTI", "MEM_TOTAL", "MEM_USED", "MEM_FREE", "DISK_TOTAL",
                            "DISK_USED", "DISK_FREE", "DISK_TPS", "DISK_WRITE", "DISK_READ", "NET_RX", "NET_TX", "NET_RX_BYTE", "NET_TX_BYTE"};
            break;
        case TABLE_TYEP_MEC_DEV_ALARM:
            kColumnNames = {"TIME_STAMP",  "ALARM_TYPE","ALARM_LEVEL", "ALARM_STATUS", "ALARM_RAISED_TIME", "ALARM_CHANGED_TIME", "ADDITION"};
            break;
        case TABLE_TYEP_RADAR_DEV_ALARM:
            kColumnNames = {"TIME_STAMP",  "DEVICE_NO_ALARM_TYPE",  "ALARM_TYPE","ALARM_LEVEL", "ALARM_STATUS", "ALARM_RAISED_TIME", "ALARM_CHANGED_TIME", "ADDITION"};
            break;
        case TABLE_TYEP_CAMERA_DEV_ALARM:
            kColumnNames = {"TIME_STAMP",  "DEVICE_NO_ALARM_TYPE", "ALARM_TYPE","ALARM_LEVEL", "ALARM_STATUS", "ALARM_RAISED_TIME", "ALARM_CHANGED_TIME", "ADDITION"};
            break;
        default:
            return false;
            break;
    }

    sqlite3_stmt* stmt;
    std::string sql = "SELECT ";
    for (const auto& columnName : kColumnNames)
    {
        sql += columnName;
        if (&columnName != &kColumnNames.back())
        {
            sql += ", ";
        }
    }
    sql += " FROM [" + tableInfo.first + "]";

    int rc = sqlite3_prepare_v2(sqlite_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        OM_DS_ERROR_PRINT << interface  << "[error]: Prepare statement [" << tableInfo.first.c_str() << "] fail, check this table exists";
//        OM_DS_ERROR_PRINT << "[error]: Prepare statement fail " << sqlite3_errmsg(sqlite_) << std::endl << sql;
        return false;
    }
    char* expandedSql = sqlite3_expanded_sql(stmt);
//    OM_DS_DEBUG_PRINT << "[sql]\n" << expandedSql;
    sqlite3_free(expandedSql);
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        MsgDeviceStatus status;
        for (size_t i = 0; i < kColumnNames.size(); ++i)
        {
            switch (sqlite3_column_type(stmt, i))
            {
                case SQLITE_FLOAT:
                    if (strcmp(kColumnNames[i].c_str(), "LONGITUDE") == 0)
                    {
                        status.longitude = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "LATITUDE") == 0)
                    {
                        status.latitude = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "ALTITUDE") == 0)
                    {
                        status.altitude = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "ALTITUDE") == 0)
                    {
                        status.altitude = sqlite3_column_double(stmt, i);
                    }

                    else if (strcmp(kColumnNames[i].c_str(), "CPU_TEMP") == 0)
                    {
                        status.cpuTemp = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "GPU_LOAD") == 0)
                    {
                        status.gpuLoad = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "GPU_SMEM") == 0)
                    {
                        status.gpuSmem = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "GPU_TEMP") == 0)
                    {
                        status.gpuTemp = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "MEM_TOTAL") == 0)
                    {
                        status.memTotal = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "MEM_USED") == 0)
                    {
                        status.memUsed = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "MEM_FREE") == 0)
                    {
                        status.memFree = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DISK_TOTAL") == 0)
                    {
                        status.diskTotal = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DISK_USED") == 0)
                    {
                        status.diskUsed = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DISK_FREE") == 0)
                    {
                        status.diskFree = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DISK_TPS") == 0)
                    {
                        status.diskTps = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DISK_WRITE") == 0)
                    {
                        status.diskWrite = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DISK_READ") == 0)
                    {
                        status.diskRead = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "NET_RX_BYTE") == 0)
                    {
                        status.netRxByte = sqlite3_column_double(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "NET_TX_BYTE") == 0)
                    {
                        status.netTxByte = sqlite3_column_double(stmt, i);
                    }
                    else
                    {

                    }
                    break;
                case SQLITE_INTEGER:
                    if (strcmp(kColumnNames[i].c_str(), "TIME_STAMP") == 0)
                    {
                        status.timeStamp = sqlite3_column_int64(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DEVICE_TYPE") == 0)
                    {
                        status.deviceType = (DEVICE_TYPE_DB)sqlite3_column_int(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DEVICE_STATUS") == 0)
                    {
                        status.deviceStatus = (DEVICE_STATUS_DB)sqlite3_column_int(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DEV_TIME") == 0)
                    {
                        status.devTime = sqlite3_column_int64(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "LAST_TIME") == 0)
                    {
                        status.lastTime = sqlite3_column_int64(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "TIME_NODE") == 0)
                    {
                        status.timeNode = sqlite3_column_int64(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "TIME_DIFF") == 0)
                    {
                        status.timeDiff = sqlite3_column_int64(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DEVICE_ACTIVE") == 0)
                    {
                        status.active = (DEVICE_ACTIVE_DB)sqlite3_column_int(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "NET_RX") == 0)
                    {
                        status.netRx = sqlite3_column_int64(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "NET_TX") == 0)
                    {
                        status.netTx = sqlite3_column_int64(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "ALARM_LEVEL") == 0)
                    {
                        status.alarmLevel = (AlertLevelEnum)sqlite3_column_int(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "ALARM_STATUS") == 0)
                    {
                        status.alarmStatus = (AlarmStatusEnum)sqlite3_column_int(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "ALARM_RAISED_TIME") == 0)
                    {
                        status.alarmRaisedTime = sqlite3_column_int64(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "ALARM_CHANGED_TIME") == 0)
                    {
                        status.alarmChangedTime = sqlite3_column_int64(stmt, i);
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "ALARM_TYPE") == 0)
                    {
                        status.alarmType = sqlite3_column_int64(stmt, i);
                    }
                    else
                    {

                    }
                    break;
                case SQLITE_TEXT:
                    if (strcmp(kColumnNames[i].c_str(), "DEVICE_ID") == 0)
                    {
                        status.deviceID = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "SOFTWARE_VERSION") == 0)
                    {
                        status.softwareVersion =  reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "POINT_NO") == 0)
                    {
                        status.pointNo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "POINT_NAME") == 0)
                    {
                        status.pointName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DEVICE_IP") == 0)
                    {
                        status.deviceIp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "CPU_LOAD") == 0)
                    {
                        status.cpuLoad = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "CPU_UTI") == 0)
                    {
                        status.cpuUti = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "NET_MASK") == 0)
                    {
                        status.netMask = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "GATEWAY") == 0)
                    {
                        status.gateway = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "GPU_UTI") == 0)
                    {
                        status.gpuUti = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "ADDITION") == 0)
                    {
                        status.addition =  reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else if (strcmp(kColumnNames[i].c_str(), "DEVICE_NO_ALARM_TYPE") == 0)
                    {
                        status.addition =  reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    }
                    else
                    {

                    }
                    break;
                default:
                    OM_DS_ERROR_PRINT << interface  << "[error]: Unsupported column type " << sqlite3_column_type(stmt, i) << " for " << kColumnNames[i].c_str();
                    break;
            }
        }
        statusList.push_back(status);
    }
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        OM_DS_ERROR_PRINT << interface  << "[error]Error reading data " << sqlite3_errmsg(sqlite_);
        return false;
    }

    return true;
}
bool DeviceStatusMsgDB::ClearTable(std::pair<std::string, TABLE_TYPE> tableInfo, std::string interface)
{
    std::string sql;
    switch(tableInfo.second)
    {
        case TABLE_TYEP_SENSOR_DEV_STATUS:  //设备状态表
            sql = "DROP TABLE IF EXISTS  [" + tableInfo.first + "]";
            break;
        case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:  //设备授时表
            sql = "DROP TABLE IF EXISTS  [" + tableInfo.first + "]";
            break;
        case TABLE_TYEP_MEC_DEV_STATUS:  //MEC状态表
            sql = "DROP TABLE IF EXISTS  [" + tableInfo.first + "]";
            break;
        case TABLE_TYEP_MEC_DEV_ALARM:  //感知告警表
            sql = "DROP TABLE IF EXISTS  [" + tableInfo.first + "]";
            break;
        case TABLE_TYEP_RADAR_DEV_ALARM:  //雷达告警表
            sql = "DROP TABLE IF EXISTS  [" + tableInfo.first + "]";
            break;
        case TABLE_TYEP_CAMERA_DEV_ALARM:  //相机告警表
            sql = "DROP TABLE IF EXISTS  [" + tableInfo.first + "]";
            break;
        default:
            break;
    }

    if(sql.empty())
    {
        return false;
    }
    if (enableDebugPrint)
    {
        OM_DS_DEBUG_PRINT << "[sql]" <<  sql.c_str();
    }

    char *sErrMsg = nullptr;
    if (sqlite3_exec(sqlite_, sql.c_str(), NULL, NULL, &sErrMsg) != SQLITE_OK)
    {
        OM_DS_ERROR_PRINT << interface << "[error]Clear datebase table " << tableInfo.first.c_str() << " fail, " << sErrMsg;
        return false;
    }
    sqlite3_free(sErrMsg);
    return true;
}
bool DeviceStatusMsgDB::deleteTableSomeLine(std::pair<std::string, TABLE_TYPE> tableInfo, const MsgDeviceStatus& status, std::string interface)
{
    std::string sql;
    switch(tableInfo.second)
    {
        case TABLE_TYEP_SENSOR_DEV_STATUS:  //设备状态表
            sql = "DELETE FROM [" + tableInfo.first + "]  WHERE DEVICE_ID = '" + status.deviceID + "'";
            break;
        case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:  //设备授时表
            sql = "DELETE FROM [" + tableInfo.first + "]  WHERE DEVICE_ID = '" + status.deviceID + "'";
            break;
        case TABLE_TYEP_MEC_DEV_STATUS:  //MEC状态表
            sql = "DELETE FROM [" + tableInfo.first + "]  WHERE DEVICE_ID = '" + status.deviceID + "'";
            break;
        case TABLE_TYEP_MEC_DEV_ALARM:  //感知告警表
            sql = "DELETE FROM [" + tableInfo.first + "]  WHERE ALARM_TYPE = " + std::to_string(status.alarmType);
            break;
        case TABLE_TYEP_RADAR_DEV_ALARM:  //雷达告警表
            sql = "DELETE FROM [" + tableInfo.first + "]  WHERE DEVICE_NO_ALARM_TYPE = '" + status.deviceNoAlarmType + "'";
            break;
        case TABLE_TYEP_CAMERA_DEV_ALARM:  //相机告警表
            sql = "DELETE FROM [" + tableInfo.first + "]  WHERE DEVICE_NO_ALARM_TYPE = '" + status.deviceNoAlarmType + "'";
            break;
        default:
            break;
    }
    
    if(sql.empty())
    {
        return false;
    }
    if (enableDebugPrint)
    {
        OM_DS_DEBUG_PRINT << "[sql]" <<  sql.c_str();
    }

    char *sErrMsg = nullptr;
    if (sqlite3_exec(sqlite_, sql.c_str(), NULL, NULL, &sErrMsg) != SQLITE_OK)
    {
        OM_DS_ERROR_PRINT << interface  << "[error]Clear datebase table line " << tableInfo.first.c_str() << " fail, " << sErrMsg;
        return false;
    }
    sqlite3_free(sErrMsg);
    return true;
}

////////////////////////////////////////////////////////////////
std::string DeviceStatusMsgDB::getTableUpdateSqlSomeField(std::pair<std::string, TABLE_TYPE> tableInfo, TABLE_OPERA_TYPE operType, const MsgDeviceStatus& status)
{
    std::string sql;
    switch(operType)
    {
        case  TABLE_OPERA_TYPE_UPDATE:  //更新操作
            switch(tableInfo.second)
            {
                case TABLE_TYEP_SENSOR_DEV_STATUS:  //感知设备状态表
                    sql = "UPDATE [" + tableInfo.first  + "] SET ";
                    if(status.timeStampNeedUpdate){sql.append("TIME_STAMP = ?, ");}
                    if(status.deviceTypeNeedUpdate){sql.append("DEVICE_TYPE = ?, ");}
//                    if(status.deviceIDNeedUpdate){sql.append("DEVICE_ID = ?, ");}
                    if(status.deviceStatusNeedUpdate){sql.append("DEVICE_STATUS = ?, ");}
                    if(status.activeNeedUpdate){sql.append("DEVICE_ACTIVE = ?, ");}
                    if(status.mecEsnNeedUpdate){sql.append("MEC_ESN = ?, ");}
                    if(status.deviceIpNeedUpdate){sql.append("DEVICE_IP = ?, ");}
                    if(status.longitudeNeedUpdate){sql.append("LONGITUDE = ?, ");}
                    if(status.latitudeNeedUpdate){sql.append("LATITUDE = ?, ");}
                    if(status.altitudeNeedUpdate){sql.append("ALTITUDE = ?, ");}


                    if(status.alarmTypeNeedUpdate){sql.append("ALARM_TYPE = ?, ");}
                    if(status.additionNeedUpdate){sql.append("ADDITION = ?, ");}

                    sql = sql.substr(0, sql.find_last_of(","));
                    sql.append( " WHERE DEVICE_ID = ?");
                    break;
                case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:  //感知设备授时表
                    sql = "UPDATE [" + tableInfo.first + "] SET ";
                    if (status.timeStampNeedUpdate) {sql.append("TIME_STAMP = ?, ");}
                    if (status.deviceTypeNeedUpdate) {sql.append("DEVICE_TYPE = ?, ");}
                    if (status.deviceStatusNeedUpdate) {sql.append("DEVICE_STATUS = ?, ");}
                    if (status.devTimeNeedUpdate) {sql.append("DEV_TIME = ?, ");}
                    if (status.lastTimeNeedUpdate) {sql.append("LAST_TIME = ?, ");}
                    if (status.timeDiffNeedUpdate) {sql.append("TIME_DIFF = ?, ");}
                    sql = sql.substr(0, sql.find_last_of(","));
                    sql.append( " WHERE DEVICE_ID = ?");
                    break;
                case TABLE_TYEP_MEC_DEV_STATUS:  //MEC状态表
                    sql = "UPDATE [" + tableInfo.first + "] SET ";
                    if (status.timeStampNeedUpdate) { sql.append("TIME_STAMP = ?, "); }
                    if (status.deviceTypeNeedUpdate) { sql.append("DEVICE_TYPE = ?, "); }
                    if (status.deviceIDNeedUpdate) { sql.append("DEVICE_ID = ?, "); }
                    if (status.deviceStatusNeedUpdate) { sql.append("DEVICE_STATUS = ?, "); }
                    if (status.devTimeNeedUpdate) { sql.append("DEV_TIME = ?, "); }
                    if (status.lastTimeNeedUpdate) { sql.append("LAST_TIME = ?, "); }
                    if (status.timeNodeNeedUpdate) { sql.append("TIME_NODE = ?, "); }
                    if (status.timeDiffNeedUpdate) { sql.append("TIME_DIFF = ?, "); }
                    if (status.pointNoNeedUpdate) { sql.append("POINT_NO = ?, "); }
                    if (status.pointNameNeedUpdate) { sql.append("POINT_NAME = ?, "); }
                    if (status.longitudeNeedUpdate) { sql.append("LON = ?, "); }
                    if (status.latitudeNeedUpdate) { sql.append("LAT = ?, "); }
                    if (status.deviceIpNeedUpdate) { sql.append("IP = ?, "); }
                    if (status.netMaskNeedUpdate) { sql.append("NET_MASK = ?, "); }
                    if (status.gatewayNeedUpdate) { sql.append("GATEWAY = ?, "); }
                    if (status.cpuLoadNeedUpdate) { sql.append("CPU_LOAD = ?, "); }
                    if (status.cpuTempNeedUpdate) { sql.append("CPU_TEMP = ?, "); }
                    if (status.cpuUtiNeedUpdate) { sql.append("CPU_UTI = ?, "); }
                    if (status.gpuLoadNeedUpdate) { sql.append("GPU_LOAD = ?, "); }
                    if (status.gpuSmemNeedUpdate) { sql.append("GPU_SMEM = ?, "); }
                    if (status.gpuTempNeedUpdate) { sql.append("GPU_TEMP = ?, "); }
                    if (status.gpuUtiNeedUpdate) { sql.append("GPU_UTI = ?, "); }
                    if (status.memTotalNeedUpdate) { sql.append("MEM_TOTAL = ?, "); }
                    if (status.memUsedNeedUpdate) { sql.append("MEM_USED = ?, "); }
                    if (status.memFreeNeedUpdate) { sql.append("MEM_FREE = ?, "); }
                    if (status.diskTotalNeedUpdate) { sql.append("DISK_TOTAL = ?, "); }
                    if (status.diskUsedNeedUpdate) { sql.append("DISK_USED = ?, "); }
                    if (status.diskFreeNeedUpdate) { sql.append("DISK_FREE = ?, "); }
                    if (status.diskTpsNeedUpdate) { sql.append("DISK_TPS = ?, "); }
                    if (status.diskWriteNeedUpdate) { sql.append("DISK_WRITE = ?, "); }
                    if (status.diskReadNeedUpdate) { sql.append("DISK_READ = ?, "); }

                    
                    if (status.netRxNeedUpdate) { sql.append("NET_RX = ?, "); }
                    if (status.netTxNeedUpdate) { sql.append("NET_TX = ?, "); }
                    if (status.netRxByteNeedUpdate) { sql.append("NET_RX_BYTE = ?, "); }
                    if (status.netTxByteNeedUpdate) { sql.append("NET_TX_BYTE = ?, "); }
                  // 移除最后一个多余的逗号
                    sql = sql.substr(0, sql.find_last_of(","));
                    sql.append(" WHERE DEVICE_ID = ?");
                    break;
                case TABLE_TYEP_MEC_DEV_ALARM:  //Mec告警表
                    sql = "UPDATE [" + tableInfo.first + "] SET ";
                    if (status.timeStampNeedUpdate) { sql.append("TIME_STAMP = ?, "); }
                    if (status.alarmLevelNeedUpdate) { sql.append("ALARM_LEVEL = ?, "); }
                    if (status.alarmStatusNeedUpdate) { sql.append("ALARM_STATUS = ?, "); }
                    if (status.alarmRaisedTimeNeedUpdate) { sql.append("ALARM_RAISED_TIME = ?, "); }
                    if (status.alarmChangedTimeNeedUpdate) { sql.append("ALARM_CHANGED_TIME = ?, "); }
                    if (status.additionNeedUpdate) { sql.append("ADDITION = ?, "); }
                    // 移除最后一个多余的逗号
                    sql = sql.substr(0, sql.find_last_of(","));
                    sql.append(" WHERE ALARM_TYPE = ?");
                    break;
                case TABLE_TYEP_RADAR_DEV_ALARM:  //雷达告警表
                    sql = "UPDATE [" + tableInfo.first + "] SET ";
                    if (status.timeStampNeedUpdate) { sql.append("TIME_STAMP = ?, "); }
                    if (status.alarmTypeNeedUpdate) { sql.append("ALARM_TYPE = ?, "); }
                    if (status.alarmLevelNeedUpdate) { sql.append("ALARM_LEVEL = ?, "); }
                    if (status.alarmStatusNeedUpdate) { sql.append("ALARM_STATUS = ?, "); }
                    if (status.alarmRaisedTimeNeedUpdate) { sql.append("ALARM_RAISED_TIME = ?, "); }
                    if (status.alarmChangedTimeNeedUpdate) { sql.append("ALARM_CHANGED_TIME = ?, "); }
                    if (status.additionNeedUpdate) { sql.append("ADDITION = ?, "); }
                    // 移除最后一个多余的逗号
                    sql = sql.substr(0, sql.find_last_of(","));
                    sql.append(" WHERE DEVICE_NO_ALARM_TYPE = ?");
                    break;
                case TABLE_TYEP_CAMERA_DEV_ALARM:  //相机告警表
                    sql = "UPDATE [" + tableInfo.first + "] SET ";
                    if (status.timeStampNeedUpdate) { sql.append("TIME_STAMP = ?, "); }
                    if (status.alarmTypeNeedUpdate) { sql.append("ALARM_TYPE = ?, "); }
                    if (status.alarmLevelNeedUpdate) { sql.append("ALARM_LEVEL = ?, "); }
                    if (status.alarmStatusNeedUpdate) { sql.append("ALARM_STATUS = ?, "); }
                    if (status.alarmRaisedTimeNeedUpdate) { sql.append("ALARM_RAISED_TIME = ?, "); }
                    if (status.alarmChangedTimeNeedUpdate) { sql.append("ALARM_CHANGED_TIME = ?, "); }
                    if (status.additionNeedUpdate) { sql.append("ADDITION = ?, "); }
                    // 移除最后一个多余的逗号
                    sql = sql.substr(0, sql.find_last_of(","));
                    sql.append(" WHERE DEVICE_NO_ALARM_TYPE = ?");
                    break;
                default:
                    break;
            }
            break;
        default:
            sql = "";
            break;
    }
    return sql;
};

bool DeviceStatusMsgDB::insertOrUpdateDeviceStatusSomeField(std::pair<std::string, TABLE_TYPE> tableInfo, const MsgDeviceStatus& status, std::string interface)
{
    sqlite3_stmt* stmt;
    int count = 0;
    std::string sql;
    try {
        if(tableInfo.second == TABLE_TYEP_MEC_DEV_ALARM )
        {
            sql = "SELECT COUNT(*) FROM [" + tableInfo.first + "] WHERE ALARM_TYPE = ?";
        }
        else if(tableInfo.second == TABLE_TYEP_RADAR_DEV_ALARM ||  tableInfo.second == TABLE_TYEP_CAMERA_DEV_ALARM)
        {
            sql = "SELECT COUNT(*) FROM [" + tableInfo.first + "] WHERE DEVICE_NO_ALARM_TYPE = ?";
        }
        else
        {
            sql = "SELECT COUNT(*) FROM [" + tableInfo.first + "] WHERE DEVICE_ID = ?";
        }
        if (enableDebugPrint)
        {
            OM_DS_DEBUG_PRINT << interface << "[sql]" << sql;
        }
//        OM_DS_ERROR_PRINT << "[table-info]" << tableInfo.first << "[type]" << tableInfo.second;
        int rc = sqlite3_prepare_v2(sqlite_, sql.c_str(), -1, &stmt, nullptr);
        if (rc == SQLITE_OK)
        {
            if(tableInfo.second == TABLE_TYEP_MEC_DEV_ALARM)
            {
//                sqlite3_bind_text(stmt, 1, status.addition.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 1, status.alarmType);
            }
            else if(tableInfo.second == TABLE_TYEP_RADAR_DEV_ALARM ||  tableInfo.second == TABLE_TYEP_CAMERA_DEV_ALARM)
            {
                sqlite3_bind_text(stmt, 1, status.deviceNoAlarmType.c_str(), -1, SQLITE_TRANSIENT);
            }
            else
            {
                sqlite3_bind_text(stmt, 1, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
            }
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                count = sqlite3_column_int(stmt, 0);
            }
            else
            {
                OM_DS_ERROR_PRINT << interface  << "[error]Check exists fail";
                rc = -1;
                sqlite3_finalize(stmt);
                sqlite3_close(sqlite_);
                return false;
            }
            //        sqlite3_finalize(stmt);
        }
        else
        {
            OM_DS_ERROR_PRINT << interface  << "[error]Check exists fail " << rc;
            rc = -1;
            sqlite3_finalize(stmt);
            sqlite3_close(sqlite_);
            return false;
        }
        if (enableDebugPrint)
        {
            OM_DS_DEBUG_PRINT << interface << "[count]" << count;
        }
        if (count > 0 )
        {
            std::string sql = getTableUpdateSqlSomeField(tableInfo, TABLE_OPERA_TYPE_UPDATE, status);
            if(sql.size() <= 0)
            {
                OM_DS_ERROR_PRINT << interface  << "[error]sql.size() <= 0";
                sqlite3_close(sqlite_);
                return false;
            }
//            OM_DS_WARN_PRINT << "[sql]" << sql;
            int rc = sqlite3_prepare_v2(sqlite_, sql.c_str(), -1, &stmt, NULL);
            if (rc == SQLITE_OK)
            {
                switch (tableInfo.second)
                {
                    case TABLE_TYEP_SENSOR_DEV_STATUS:
                    {
                        int index = 1;
                        if(status.timeStampNeedUpdate){  sqlite3_bind_int64(stmt, index++, status.timeStamp); }
                        if(status.deviceTypeNeedUpdate){ sqlite3_bind_int(stmt, index++, status.deviceType); }
                        if(status.deviceStatusNeedUpdate){ sqlite3_bind_int(stmt, index++, status.deviceStatus); }
                        if(status.activeNeedUpdate){ sqlite3_bind_int(stmt, index++, status.active); }
                        if(status.mecEsnNeedUpdate){  sqlite3_bind_text(stmt, index++, status.mecEsn.c_str(), -1, SQLITE_TRANSIENT); }
                        if(status.deviceIpNeedUpdate){  sqlite3_bind_text(stmt, index++, status.deviceIp.c_str(), -1, SQLITE_TRANSIENT); }
                        if(status.longitudeNeedUpdate){ sqlite3_bind_double(stmt, index++, status.longitude); }
                        if(status.latitudeNeedUpdate){  sqlite3_bind_double(stmt, index++, status.latitude); }
                        if(status.altitudeNeedUpdate){ sqlite3_bind_double(stmt, index++, status.altitude); }
                        
                        if(status.alarmTypeNeedUpdate){ sqlite3_bind_int(stmt, index++, status.alarmType); }
                        if(status.additionNeedUpdate){ sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT); }
                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
                    }
                    break;
                    case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:
                    {
                        int index = 1;
                        if(status.timeStampNeedUpdate){  sqlite3_bind_int64(stmt, index++, status.timeStamp); }
                        if(status.deviceTypeNeedUpdate){ sqlite3_bind_int(stmt, index++, status.deviceType); }
                        if(status.deviceStatusNeedUpdate){ sqlite3_bind_int(stmt, index++, status.deviceStatus); }
                        if (status.devTimeNeedUpdate) {  sqlite3_bind_int64(stmt, index++, status.devTime); }
                        if (status.lastTimeNeedUpdate) { sqlite3_bind_int64(stmt, index++, status.lastTime); }
                        if (status.timeDiffNeedUpdate) { sqlite3_bind_int64(stmt, index++, status.timeDiff); }
                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
                    }
                    break;
                    case TABLE_TYEP_MEC_DEV_STATUS:
                    {
                        int index = 1; // Reset index
                        if (status.timeStampNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.timeStamp);
                        }
                        if (status.deviceTypeNeedUpdate) {
                            sqlite3_bind_int(stmt, index++, status.deviceType);
                        }
                        if (status.deviceIDNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        if (status.deviceStatusNeedUpdate) {
                            sqlite3_bind_int(stmt, index++, status.deviceStatus);
                        }
                        if (status.devTimeNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.devTime);
                        }
                        if (status.lastTimeNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.lastTime);
                        }
                        if (status.timeNodeNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.timeNode);
                        }
                        if (status.timeDiffNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.timeDiff);
                        }
                        if (status.pointNoNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.pointNo.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        if (status.pointNameNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.pointName.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        if (status.longitudeNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.longitude);
                        }
                        if (status.latitudeNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.latitude);
                        }
                        if (status.deviceIpNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.deviceIp.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        if (status.netMaskNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.netMask.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        if (status.gatewayNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.gateway.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        if (status.cpuLoadNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.cpuLoad.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        if (status.cpuTempNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.cpuTemp);
                        }
                        if (status.cpuUtiNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.cpuUti.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        if (status.gpuLoadNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.gpuLoad);
                        }
                        if (status.gpuSmemNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.gpuSmem);
                        }
                        if (status.gpuTempNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.gpuTemp);
                        }
                        if (status.gpuUtiNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.gpuUti.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        if (status.memTotalNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.memTotal);
                        }
                        if (status.memUsedNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.memUsed);
                        }
                        if (status.memFreeNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.memFree);
                        }
                        if (status.diskTotalNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.diskTotal);
                        }
                        if (status.diskUsedNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.diskUsed);
                        }
                        if (status.diskFreeNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.diskFree);
                        }
                        if (status.diskTpsNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.diskTps);
                        }
                        if (status.diskWriteNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.diskWrite);
                        }
                        if (status.diskReadNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.diskRead);
                        }
                        if (status.netRxNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.netRx);
                        }
                        if (status.netTxNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.netTx);
                        }
                        if (status.netRxByteNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.netRxByte);
                        }
                        if (status.netTxByteNeedUpdate) {
                            sqlite3_bind_double(stmt, index++, status.netTxByte);
                        }
                        // 最后绑定设备 ID
                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
                    }
                    break;
                    case TABLE_TYEP_MEC_DEV_ALARM:
                    {
                        int index = 1; // Reset index
                        if (status.timeStampNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.timeStamp);
                        }
//                        if (status.alarmTypeNeedUpdate) {
//                            sqlite3_bind_int(stmt, index++, status.alarmType);
//                        }
                        if (status.alarmLevelNeedUpdate) {
                            sqlite3_bind_int(stmt, index++, status.alarmLevel);
                        }
                        if (status.alarmStatusNeedUpdate) {
                            sqlite3_bind_int(stmt, index++, status.alarmStatus);
                        }
                        if (status.alarmRaisedTimeNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);
                        }
                        if (status.alarmChangedTimeNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);
                        }
						if (status.additionNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        // 最后绑定告警类型
                        sqlite3_bind_int(stmt, index++, status.alarmType);
                    }

                    break;
                    case TABLE_TYEP_RADAR_DEV_ALARM:  //雷达设备告警
                    {
                        int index = 1; // Reset index
                        if (status.timeStampNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.timeStamp);
                        }
                        if (status.alarmTypeNeedUpdate) {
                            sqlite3_bind_int(stmt, index++, status.alarmType);
                        }
                        if (status.alarmLevelNeedUpdate) {
                            sqlite3_bind_int(stmt, index++, status.alarmLevel);
                        }
                        if (status.alarmStatusNeedUpdate) {
                            sqlite3_bind_int(stmt, index++, status.alarmStatus);
                        }
                        if (status.alarmRaisedTimeNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);
                        }
                        if (status.alarmChangedTimeNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);
                        }
                        if (status.additionNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        // 最后绑定设备 ID
                        sqlite3_bind_text(stmt, index++, status.deviceNoAlarmType.c_str(), -1, SQLITE_TRANSIENT);
                    }
                    break;
                    case TABLE_TYEP_CAMERA_DEV_ALARM:  //相机设备告警
                    {
                        int index = 1; // Reset index
                        if (status.timeStampNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.timeStamp);
                        }
                        if (status.alarmTypeNeedUpdate) {
                            sqlite3_bind_int(stmt, index++, status.alarmType);
                        }
                        if (status.alarmLevelNeedUpdate) {
                            sqlite3_bind_int(stmt, index++, status.alarmLevel);
                        }
                        if (status.alarmStatusNeedUpdate) {
                            sqlite3_bind_int(stmt, index++, status.alarmStatus);
                        }
                        if (status.alarmRaisedTimeNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);
                        }
                        if (status.alarmChangedTimeNeedUpdate) {
                            sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);
                        }
                        if (status.additionNeedUpdate) {
                            sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT);
                        }
                        // 最后绑定设备 ID
                        sqlite3_bind_text(stmt, index++, status.deviceNoAlarmType.c_str(), -1, SQLITE_TRANSIENT);
                    }
                        break;
                    default:
                        break;
                }

                if(!stmt)
                {
                    OM_DS_ERROR_PRINT << interface  << "[error]stmt is null!";
                    sqlite3_close(sqlite_);
                    return false;
                }
                char* expandedSql = sqlite3_expanded_sql(stmt);
                if (!expandedSql)
                {
                    OM_DS_ERROR_PRINT << interface  << "[error]: failed to get expanded SQL!";
                    return false;
                }
                if (enableDebugPrint)
                {
                    OM_DS_DEBUG_PRINT << interface  << "[sql]" << expandedSql;
                }
                sqlite3_free(expandedSql);
                rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE)
                {
                    OM_DS_ERROR_PRINT << interface  << "[error]: Update fail " << rc;
                    rc = -1;
                    sqlite3_finalize(stmt);
                    sqlite3_close(sqlite_);
                    return false;
                }
                if (enableDebugPrint)
                {
                    OM_DS_SUCCESS_PRINT << interface  << SUCESS_COLOR_STR << "[sucess]update [" << tableInfo.first << "] success!" << SUCESS_COLOR_END;
                }
                sqlite3_finalize(stmt);
            }
            else
            {
                OM_DS_ERROR_PRINT << interface  << "[error]: Update fail " << rc;
                rc = -1;
                sqlite3_close(sqlite_);
                return false;
            }
        }
        else
        {
            std::string sql = getTableUpdateSql(tableInfo, TABLE_OPERA_TYPE_INSERT, status);
            if(sql.size() <= 0)
            {
                sqlite3_close(sqlite_);
                return false;
            }
            int rc = sqlite3_prepare_v2(sqlite_, sql.c_str(), -1, &stmt, NULL);
            if (rc == SQLITE_OK)
            {
                switch (tableInfo.second)
                {
                    case TABLE_TYEP_SENSOR_DEV_STATUS:
                    {
                        int index = 1;
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);
                        sqlite3_bind_int(stmt, index++, status.deviceType);
                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_bind_int(stmt, index++, status.deviceStatus);
                        sqlite3_bind_int(stmt, index++, status.active);
                        sqlite3_bind_text(stmt, index++, status.mecEsn.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_bind_text(stmt, index++, status.deviceIp.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_bind_double(stmt, index++, status.longitude);
                        sqlite3_bind_double(stmt, index++, status.latitude);
                        sqlite3_bind_double(stmt, index++, status.altitude);
                        sqlite3_bind_int(stmt, index++, status.alarmType);
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT);
                    }
                    break;
                    case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:
                    {
                        int index = 1;
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);
                        sqlite3_bind_int(stmt, index++, status.deviceType);
                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT);
                        sqlite3_bind_int(stmt, index++, status.deviceStatus);
                        sqlite3_bind_int64(stmt, index++, status.devTime);
                        sqlite3_bind_int64(stmt, index++, status.lastTime);
                        sqlite3_bind_int64(stmt, index++, status.timeDiff);
                    }
                    break;
                    case TABLE_TYEP_MEC_DEV_STATUS:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_text(stmt, index++, status.deviceID.c_str(), -1, SQLITE_TRANSIENT); // DEVICE_ID
                        sqlite3_bind_int(stmt, index++, status.deviceType);           // DEVICE_TYPE
                        sqlite3_bind_int(stmt, index++, status.deviceStatus);         // DEVICE_STATUS
                        sqlite3_bind_int(stmt, index++, status.active);               // DEVICE_ACTIVE
                        sqlite3_bind_text(stmt, index++, status.softwareVersion.c_str(), -1, SQLITE_TRANSIENT); // SOFTWARE_VERSION
                        sqlite3_bind_int64(stmt, index++, status.lastTime);           // LAST_TIME
                        sqlite3_bind_int64(stmt, index++, status.timeNode);           // TIME_NODE
                        sqlite3_bind_int(stmt, index++, status.timeDiff);       // TIME_DIFFERENCE
                        sqlite3_bind_text(stmt, index++, status.pointNo.c_str(), -1, SQLITE_TRANSIENT);              // POINT_NO
                        sqlite3_bind_text(stmt, index++, status.pointName.c_str(), -1, SQLITE_TRANSIENT); // POINT_NAME
                        sqlite3_bind_double(stmt, index++, status.longitude);         // LON
                        sqlite3_bind_double(stmt, index++, status.latitude);          // LAT
                        sqlite3_bind_text(stmt, index++, status.deviceIp.c_str(), -1, SQLITE_TRANSIENT); // IP
                        sqlite3_bind_text(stmt, index++, status.netMask.c_str(), -1, SQLITE_TRANSIENT);   // NET_MASK
                        sqlite3_bind_text(stmt, index++, status.gateway.c_str(), -1, SQLITE_TRANSIENT);     // GATEWAY
                        sqlite3_bind_text(stmt, index++, status.cpuLoad.c_str(), -1, SQLITE_TRANSIENT);               // CPU_LOAD
                        sqlite3_bind_double(stmt, index++, status.cpuTemp);              // CPU_TEMP
                        sqlite3_bind_text(stmt, index++, status.cpuUti.c_str(), -1, SQLITE_TRANSIENT);           // CPU_UTI
                        sqlite3_bind_double(stmt, index++, status.gpuLoad);              // GPU_LOAD
                        sqlite3_bind_double(stmt, index++, status.gpuSmem);              // GPU_SMEM
                        sqlite3_bind_double(stmt, index++, status.gpuTemp);              // GPU_TEMP
                        sqlite3_bind_text(stmt, index++, status.gpuUti.c_str(), -1, SQLITE_TRANSIENT);               // GPU_UTI
                        sqlite3_bind_double(stmt, index++, status.memTotal);           // MEM_TOTAL
                        sqlite3_bind_double(stmt, index++, status.memUsed);            // MEM_USED
                        sqlite3_bind_double(stmt, index++, status.memFree);            // MEM_FREE
                        sqlite3_bind_double(stmt, index++, status.diskTotal);          // DISK_TOTAL
                        sqlite3_bind_double(stmt, index++, status.diskUsed);           // DISK_USED
                        sqlite3_bind_double(stmt, index++, status.diskFree);           // DISK_FREE
                        sqlite3_bind_double(stmt, index++, status.diskTps);              // DISK_TPS
                        sqlite3_bind_double(stmt, index++, status.diskWrite);          // DISK_WRITE
                        sqlite3_bind_double(stmt, index++, status.diskRead);           // DISK_READ
                        
                        sqlite3_bind_int64(stmt, index++, status.netRx);           // netRx
                        sqlite3_bind_int64(stmt, index++, status.netTx);           // netTx
                        sqlite3_bind_double(stmt, index++, status.netRxByte);          // netRxByte
                        sqlite3_bind_double(stmt, index++, status.netTxByte);           // netTxByte
                    }
                    break;
                    case TABLE_TYEP_MEC_DEV_ALARM:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_int(stmt, index++, status.alarmType);           //  ALARM_TYPE
                        sqlite3_bind_int(stmt, index++, status.alarmLevel);           // ALARM_LEVEL
                        sqlite3_bind_int(stmt, index++, status.alarmStatus);          // ALARM_STATUS
                        sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);    // ALARM_RAISED_TIME
                        sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);   // ALARM_CHANGED_TIME
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT); //
                    }
                    break;
                    case TABLE_TYEP_RADAR_DEV_ALARM:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_text(stmt, index++, status.deviceNoAlarmType.c_str(), -1, SQLITE_TRANSIENT); //
                        sqlite3_bind_int(stmt, index++, status.alarmType);           //  ALARM_TYPE
                        sqlite3_bind_int(stmt, index++, status.alarmLevel);           // ALARM_LEVEL
                        sqlite3_bind_int(stmt, index++, status.alarmStatus);          // ALARM_STATUS
                        sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);    // ALARM_RAISED_TIME
                        sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);   // ALARM_CHANGED_TIME
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT); //
                    }
                    break;
                    case TABLE_TYEP_CAMERA_DEV_ALARM:
                    {
                        int index = 1; // SQLite binds values starting at index 1
                        sqlite3_bind_int64(stmt, index++, status.timeStamp);          // TIME_STAMP
                        sqlite3_bind_text(stmt, index++, status.deviceNoAlarmType.c_str(), -1, SQLITE_TRANSIENT); //
                        sqlite3_bind_int(stmt, index++, status.alarmType);           //  ALARM_TYPE
                        sqlite3_bind_int(stmt, index++, status.alarmLevel);           // ALARM_LEVEL
                        sqlite3_bind_int(stmt, index++, status.alarmStatus);          // ALARM_STATUS
                        sqlite3_bind_int64(stmt, index++, status.alarmRaisedTime);    // ALARM_RAISED_TIME
                        sqlite3_bind_int64(stmt, index++, status.alarmChangedTime);   // ALARM_CHANGED_TIME
                        sqlite3_bind_text(stmt, index++, status.addition.c_str(), -1, SQLITE_TRANSIENT); //
                    }
                        break;
                    default:
                        break;
                }
                if(!stmt)
                {
                    OM_DS_ERROR_PRINT << interface  << "[error]stmt is null!";
                    return false;
                }
                char* expandedSql = sqlite3_expanded_sql(stmt);
                if (enableDebugPrint)
                {
                    OM_DS_DEBUG_PRINT << interface  << "[sql]" << expandedSql;
                }
                sqlite3_free(expandedSql);
                rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE)
                {
                    OM_DS_ERROR_PRINT << interface  << "[error]: Update fail " << rc;
                    rc = -1;
                    return false;
                }
                if (enableDebugPrint)
                {
                    OM_DS_SUCCESS_PRINT << interface  << SUCESS_COLOR_STR << "[sucess]insert [" << tableInfo.first << "] success!" << SUCESS_COLOR_END;
                }
                sqlite3_finalize(stmt);
            }
        }
    }
    catch (afl::util::Exception& e)
    {
        OM_DS_ERROR_PRINT << interface  << "[error]" << e.what();
    }
    

    return true;
}

NAMESPACE_ENDED_OM_COMPONENT_DEVICE_STATUS