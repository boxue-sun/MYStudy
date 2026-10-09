/*********************************************************************************
 * @file		configer_radar_static
 * @brief		configer_radar_static belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-12-3
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-12-3 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef MIDDLEWART_PROTOCOL_CCINDEX_DATA_MODEL_CONFIG_DATA
#define MIDDLEWART_PROTOCOL_CCINDEX_DATA_MODEL_CONFIG_DATA

#include "middleware/protocol/om_common/namespace.h"

NAMESPACE_START_RADAR_RADAR_STATIC
using namespace afl::base;
enum RADAR_STATIC_MESSAGE_TYPE{
    RadarStaticUnkown = 0,
    RadarStaticQueryAck=1,
    RadarStaticUpdate=2,
};

struct ConfigDeviceData : public afl::base::SerializableData
{
    std::string device_id;
    std::string type;

    virtual void serialize(json& j) override
    {
        JsonSerialize(device_id, "device_id", j, false);
        JsonSerialize(type, "type", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(device_id, "device_id", j, noUse_isEmptyFlag);
        JsonDeserialize(type, "type", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "device_id: " << device_id <<  std::endl;
        ss << std::left << std::setw(40) << "type: "      << type      <<  std::endl;
        return ss.str();
    }
};
struct ConfigLaneDeviceData : public afl::base::SerializableData
{
    std::string device_id;
    std::string type;
    int         section;

    virtual void serialize(json& j) override
    {
        JsonSerialize(device_id, "device_id", j, false);
        JsonSerialize(type, "type", j, false);
        JsonSerialize(section, "section", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(device_id, "device_id", j, noUse_isEmptyFlag);
        JsonDeserialize(type, "type", j, noUse_isEmptyFlag);
        JsonDeserialize(section, "section", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "device_id: " << device_id << std::endl;
        ss << std::left << std::setw(40) << "type: "      << type      << std::endl;
        ss << std::left << std::setw(40) << "section: "   << section   << std::endl;
        return ss.str();
    }
};
struct ConfigLaneData : public afl::base::SerializableData
{
    string lane_no;
    int32_t sequence;
    std::vector<ConfigDeviceData> devices; // 数组类型

    virtual void serialize(json& j) override
    {
        JsonSerialize(lane_no, "lane_no", j, false);
        JsonSerialize(sequence, "sequence", j, false);
        JsonSerialize(devices, "devices", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(lane_no, "lane_no", j, noUse_isEmptyFlag);
        JsonDeserialize(sequence, "sequence", j, noUse_isEmptyFlag);
        JsonDeserialize(devices, "devices", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "lane_no: "                      << lane_no                     << std::endl;
        ss << std::left << std::setw(40) << "sequence: "                     << sequence                    << std::endl;

        ss << std::left << std::setw(40) << "devices: "                    << std::endl; // 开始输出设备数组
        for (const auto& device : devices) {
            ss << device.to_string() << std::endl; // 假设ConfigDeviceData有to_string()方法
        }

        return ss.str();
    }
};
struct ConfigBranchData : public afl::base::SerializableData
{
    string  branch_no;
    int32_t direction;
    int32_t attribute;
    std::vector<ConfigLaneData> lanes; // 数组类型
    std::vector<ConfigDeviceData> devices; // 数组类型

    virtual void serialize(json& j) override
    {
        JsonSerialize(branch_no, "branch_no", j, false);
        JsonSerialize(direction, "direction", j, false);
        JsonSerialize(attribute, "attribute", j, false);
        JsonSerialize(lanes, "lanes", j, false);
        JsonSerialize(devices, "devices", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(branch_no, "branch_no", j, noUse_isEmptyFlag);
        JsonDeserialize(direction, "direction", j, noUse_isEmptyFlag);
        JsonDeserialize(attribute, "attribute", j, noUse_isEmptyFlag);
        JsonDeserialize(lanes, "lanes", j, noUse_isEmptyFlag);
        JsonDeserialize(devices, "devices", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "branch_no: "           << branch_no           << std::endl;
        ss << std::left << std::setw(40) << "direction: "           << direction           << std::endl;
        ss << std::left << std::setw(40) << "attribute: "           << attribute           << std::endl;
        ss << std::left << std::setw(40) << "lanes: "               << lanes.size()        << " lanes" << std::endl;
        ss << std::left << std::setw(40) << "lanes: " << std::endl; // 开始输出设备数组
        for (const auto& lane : lanes) {
            ss << lane.to_string() << std::endl;
        }
        ss << std::left << std::setw(40) << "devices: " << std::endl; // 开始输出设备数组
        for (const auto& device : devices) {
            ss << device.to_string() << std::endl;
        }

        return ss.str();
    }
};

struct ConfigIntersectionData : public afl::base::SerializableData
{
    std::string place_no;
    std::vector<ConfigBranchData> branches; // 数组类型
    std::vector<ConfigDeviceData> devices; // 数组类型
    int64_t update_time; // 可以根据需要选择使用 int64_t 或 std::string

    virtual void serialize(json& j) override
    {
        JsonSerialize(place_no, "place_no", j, false);
        JsonSerialize(branches, "branches", j, false);
        JsonSerialize(devices, "devices", j, false);
        JsonSerialize(update_time, "update_time", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(place_no, "place_no", j, noUse_isEmptyFlag);
        JsonDeserialize(branches, "branches", j, noUse_isEmptyFlag);
        JsonDeserialize(devices, "devices", j, noUse_isEmptyFlag);
        JsonDeserialize(update_time, "update_time", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "place_no: "           << place_no           << std::endl;
        ss << std::left << std::setw(40) << "branches: " << std::endl; // 开始输出设备数组
        for (const auto& branche : branches) {
            ss << branche.to_string() << std::endl;
        }
        ss << std::left << std::setw(40) << "devices: " << std::endl; // 开始输出设备数组
        for (const auto& device : devices) {
            ss << device.to_string() << std::endl;
        }
        ss << std::left << std::setw(40) << "update_time: "           << update_time           << std::endl;
        return ss.str();
    }
};
struct ConfigQueryData : public afl::base::SerializableData
{
    int32_t code;
    std::string message;
    std::vector<ConfigIntersectionData> datas; // 数组类型

    virtual void serialize(json& j) override
    {
        JsonSerialize(code, "code", j, false);
        JsonSerialize(message, "message", j, false);
        JsonSerialize(datas, "data", j, false); // 假设你有处理数组的序列化函数
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(code, "code", j, noUse_isEmptyFlag);
        JsonDeserialize(message, "message", j, noUse_isEmptyFlag);
        JsonDeserialize(datas, "data", j, noUse_isEmptyFlag); // 假设你有处理数组的反序列化函数
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "code: "    << code     << std::endl;
        ss << std::left << std::setw(40) << "message: " << message  << std::endl; // 开始输出设备数组
        for (const auto& data : datas) {
            ss << data.to_string() << std::endl;
        }
        return ss.str();
    }
};

struct ConfigUpdateData : public afl::base::SerializableData
{
    std::string cross_id;
    std::string vendor;
    std::string category;
    int32_t replace;
    std::vector<ConfigIntersectionData> datas; // 数组类型

    virtual void serialize(json& j) override
    {
        JsonSerialize(cross_id, "cross_id", j, false);
        JsonSerialize(vendor, "vendor", j, false);
        JsonSerialize(category, "category", j, false);
        JsonSerialize(replace, "replace", j, false);
        JsonSerialize(datas, "data", j, false); // 假设你有处理数组的序列化函数
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(cross_id, "cross_id", j, noUse_isEmptyFlag);
        JsonDeserialize(vendor, "vendor", j, noUse_isEmptyFlag);
        JsonDeserialize(category, "category", j, noUse_isEmptyFlag);
        JsonDeserialize(replace, "replace", j, noUse_isEmptyFlag);
        JsonDeserialize(datas, "data", j, noUse_isEmptyFlag); // 假设你有处理数组的反序列化函数
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "cross_id: "    << cross_id     << std::endl;
        ss << std::left << std::setw(40) << "vendor: "    << vendor     << std::endl;
        ss << std::left << std::setw(40) << "category: "    << category     << std::endl;
        ss << std::left << std::setw(40) << "replace: "    << replace     << std::endl;
        ss << std::left << std::setw(40) << "data: "<< std::endl; // 开始输出设备数组
        for (const auto& data : datas) {
            ss << data.to_string() << std::endl;
        }
        return ss.str();
    }
};

NAMESPACE_ENDED_RADAR_RADAR_STATIC

#endif