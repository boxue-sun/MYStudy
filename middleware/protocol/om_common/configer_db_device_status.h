/*********************************************************************************
 * @file		configer_db_device_status.h
 * @brief		configer_db_device_status belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-6-19
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-6-19 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS2_0_CONFIGER_DB_DEVICE_STAUTS_H
#define AIROS2_0_CONFIGER_DB_DEVICE_STAUTS_H
#include "namespace.h"
#include "configer_common.h"

NAMESPACE_START_OM_COMPONENT_COMMON
using namespace airos::base::workparam;
using namespace os::v2x::protocol::om::common;
struct DBConfiger : public afl::base::SerializableData
{
public:
    std::string DbFileName = "device-status.db";
    std::string DbFilePath = "/home/airos/protocol/device-status.db";
    std::string DbDeviceStatusTableName = "sensor-status";


    std::string DbSynchronizeTableName = "sensor-synchronize";
    std::string DbMecDeviceStatusTableName = "mec-status";
    std::string DbMecDeviceAlarmTableName = "mec-alarm";
    std::string DbRadarDeviceAlarmTableName = "radar-alarm";
    std::string DbCameraDeviceAlarmTableName = "camera-alarm";
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(DbFileName, "A_DbFileName", j, false);
        JsonSerialize(DbFilePath, "B_DbFilePath", j, false);
        JsonSerialize(DbDeviceStatusTableName, "C_DbDeviceStatusTableName", j, false);
        JsonSerialize(DbSynchronizeTableName, "D_DbSynchronizeTableName", j, false);
        JsonSerialize(DbMecDeviceStatusTableName, "E_DbMecDeviceStatusTableName", j, false);
        JsonSerialize(DbMecDeviceAlarmTableName, "F_DbMecDeviceAlarmTableName", j, false);
        JsonSerialize(DbRadarDeviceAlarmTableName, "G_DbRadarDeviceAlarmTableName", j, false);
        JsonSerialize(DbCameraDeviceAlarmTableName, "H_DbCameraDeviceAlarmTableName", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(DbFileName, "A_DbFileName", j, noUse_isEmptyFlag);
        JsonDeserialize(DbFilePath, "B_DbFilePath", j, noUse_isEmptyFlag);
        JsonDeserialize(DbDeviceStatusTableName, "C_DbDeviceStatusTableName", j, noUse_isEmptyFlag);
        JsonDeserialize(DbSynchronizeTableName, "D_DbSynchronizeTableName", j, noUse_isEmptyFlag);
        JsonDeserialize(DbMecDeviceStatusTableName, "E_DbMecDeviceStatusTableName", j, noUse_isEmptyFlag);
        JsonDeserialize(DbMecDeviceAlarmTableName, "F_DbMecDeviceAlarmTableName", j, noUse_isEmptyFlag);
        JsonDeserialize(DbRadarDeviceAlarmTableName, "G_DbRadarDeviceAlarmTableName", j, noUse_isEmptyFlag);
        JsonDeserialize(DbCameraDeviceAlarmTableName, "H_DbCameraDeviceAlarmTableName", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_DbFileName: " << DbFileName <<  std::endl;
        ss << std::left << std::setw(40) << "B_DbFilePath: " << DbFilePath <<  std::endl;
        ss << std::left << std::setw(40) << "C_DbDeviceStatusTableName: " << DbDeviceStatusTableName <<  std::endl;
        ss << std::left << std::setw(40) << "D_DbSynchronizeTableName: " << DbSynchronizeTableName <<  std::endl;
        ss << std::left << std::setw(40) << "E_DbMecDeviceStatusTableName: " << DbMecDeviceStatusTableName <<  std::endl;
        ss << std::left << std::setw(40) << "F_DbMecDeviceAlarmTableName: " << DbMecDeviceAlarmTableName <<  std::endl;
        ss << std::left << std::setw(40) << "G_DbSensorDeviceAlarmTableName: " << DbRadarDeviceAlarmTableName <<  std::endl;
        ss << std::left << std::setw(40) << "H_DbCameraDeviceAlarmTableName: " << DbCameraDeviceAlarmTableName <<  std::endl;

        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_COMMON
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
