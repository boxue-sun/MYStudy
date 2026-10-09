/*********************************************************************************
 * @file		alarm_management_data.h
 * @brief		alarm_management_data belongs to CICTCI
 * @details		
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-19
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  	24-4-19  alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef MQTT_CLIENT_MEC_OM_MONITOR_DATA_RADAR_CLOUD_STATUS_H
#define MQTT_CLIENT_MEC_OM_MONITOR_DATA_RADAR_CLOUD_STATUS_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/data_model/data_common.h"

NAMESPACE_START_OM_COMPONENT_MONITOR
using namespace afl::base;
using namespace os::v2x::protocol::om::common;
struct CcindexTcStatus: public afl::base::SerializableData
{
    bool tc_pub_flag;           //接收到的指标数据上云是否成功，true:成功，false：失败
    bool tc_sub_flag;

    // 序列化函数
    void serialize(json &j) override {
        JsonSerialize(tc_pub_flag,              "tc_pub_flag",              j, false);
        JsonSerialize(tc_sub_flag,       "tc_sub_flag",       j, false);
    }

    // 反序列化函数
    void deserialize(const json &j) override {
        JsonDeserialize(tc_pub_flag,              "tc_pub_flag",                     j, noUse_isEmptyFlag);
        JsonDeserialize(tc_sub_flag,       "tc_sub_flag",              j, noUse_isEmptyFlag);
    }

    // 转换为字符串
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "tc_pub_flag: "         << (tc_pub_flag ? "true" : "false") << std::endl;
        ss << std::left << std::setw(40) << "tc_sub_flag: "         << (tc_sub_flag ? "true" : "false") << std::endl;
        return ss.str();
    }
};

struct CcindexTmStatus : public afl::base::SerializableData {
    string  device_esn;
    string device_sn;
    bool tm_pub_flag;
    bool tm_sub_flag;
    // 序列化函数
    virtual void serialize(json &j) override {
        JsonSerialize(device_esn,                    "device_esn",                    j, false);
        JsonSerialize(device_sn,                  "device_sn",                  j, false);

        JsonSerialize(tm_pub_flag,                    "tm_pub_flag",                    j, false);
        JsonSerialize(tm_sub_flag,                  "tm_sub_flag",                  j, false);
    }

    // 反序列化函数
    virtual void deserialize(const json &j) override {
        JsonDeserialize(device_esn,                    "device_esn",                    j, noUse_isEmptyFlag);
        JsonDeserialize(device_sn,                  "device_sn",                  j, noUse_isEmptyFlag);

        JsonDeserialize(tm_pub_flag,                    "tm_pub_flag",                    j, noUse_isEmptyFlag);
        JsonDeserialize(tm_sub_flag,                  "tm_sub_flag",                  j, noUse_isEmptyFlag);
    }

    // 转换为字符串
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "device_esn: "                      << device_esn << std::endl;
        ss << std::left << std::setw(40) << "device_sn: "                   << device_sn << std::endl;
        ss << std::left << std::setw(40) << "tm_pub_flag: "                   << tm_pub_flag << std::endl;
        ss << std::left << std::setw(40) << "tm_sub_flag: "                   << tm_sub_flag << std::endl;
        return ss.str();
    }
};
struct CcindexStaticStatus: public afl::base::SerializableData
{
    bool st_post_flag;           //接收到的指标数据上云是否成功，true:成功，false：失败
    bool st_get_flag;

    // 序列化函数
    void serialize(json &j) override {
        JsonSerialize(st_post_flag,              "st_post_flag",              j, false);
        JsonSerialize(st_get_flag,       "st_get_flag",       j, false);
    }

    // 反序列化函数
    void deserialize(const json &j) override {
        JsonDeserialize(st_post_flag,              "st_post_flag",                     j, noUse_isEmptyFlag);
        JsonDeserialize(st_get_flag,       "st_get_flag",              j, noUse_isEmptyFlag);
    }

    // 转换为字符串
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "st_post_flag: "         << (st_post_flag ? "true" : "false") << std::endl;
        ss << std::left << std::setw(40) << "st_get_flag: "         << (st_get_flag ? "true" : "false") << std::endl;
        return ss.str();
    }
};
struct CcindexStatusResponseData : public afl::base::SerializableData
{
    bool            con_flag;                            //信控上云模块连接云控是否正常，true：连接正常，false：连接不正常
    CcindexTcStatus  ccindex_tc_status;                // MEC与云的状态
    std::vector<CcindexTmStatus>  ccindex_tm_status;                 //
    CcindexStaticStatus ccindex_static_status;

    // 序列化函数
    virtual void serialize(json &j) override
    {
        JsonSerialize(con_flag,                        "con_flag",                        j, false);
        JsonSerialize(ccindex_tc_status,                  "ccindex_tc_status",                    j, false);
        JsonSerialize(ccindex_tm_status,                 "ccindex_tm_status",                  j, false);
        JsonSerialize(ccindex_static_status,                 "ccindex_static_status",                  j, false);
    }

    // 反序列化函数
    virtual void deserialize(const json &j) override {
        JsonDeserialize(con_flag,                        "con_flag",                        j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tc_status,                  "ccindex_tc_status",                    j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_tm_status,                 "ccindex_tm_status",                  j, noUse_isEmptyFlag);
        JsonDeserialize(ccindex_static_status,                 "ccindex_static_status",                  j, noUse_isEmptyFlag);
    }

    // 转换为字符串
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "con_flag: "                         << con_flag << std::endl;
        ss << std::left << std::setw(40) << "ccindex_tc_status: "                      << ccindex_tc_status.to_string() << std::endl;
        for(auto ccindex_tm_status_temp:ccindex_tm_status)
        {
            ss << std::left << std::setw(40) << "ccindex_tm_status: "                      << ccindex_tm_status_temp.to_string() << std::endl;
        }

        ss << std::left << std::setw(40) << "ccindex_static_status: "                      << ccindex_static_status.to_string() << std::endl;
        return ss.str();
    }
};

struct CcindexStatusResponse : public afl::base::SerializableData {
    QUERY_MSG_TAG   tag;                             // 接口号
    std::string     seqnum;                            // 32位不重复的随机数
    uint64_t        timestamp;                            // utc时间戳，毫秒
    std::string     device_esn;                        // 设备esn号
    CcindexStatusResponseData data;                  // 消息结构

    // 序列化函数
    virtual void serialize(json &j) override {
        JsonSerialize(tag,                        "tag",                        j, false);
        JsonSerialize(seqnum,                    "seqnum",                    j, false);
        JsonSerialize(timestamp,                  "timestamp",                  j, false);
        JsonSerialize(device_esn,                 "device_esn",                 j, false);
        JsonSerialize(data,                 "data",                 j, false);
    }

    // 反序列化函数
    virtual void deserialize(const json &j) override {
        JsonDeserialize(tag,                        "tag",                        j, noUse_isEmptyFlag);
        JsonDeserialize(seqnum,                    "seqnum",                    j, noUse_isEmptyFlag);
        JsonDeserialize(timestamp,                  "timestamp",                  j, noUse_isEmptyFlag);
        JsonDeserialize(device_esn,                 "device_esn",                 j, noUse_isEmptyFlag);
        JsonDeserialize(data,                 "data",                 j, noUse_isEmptyFlag);
    }

    // 转换为字符串
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "tag: "                         << tag << std::endl;
        ss << std::left << std::setw(40) << "seqnum: "                      << seqnum << std::endl;
        ss << std::left << std::setw(40) << "timestamp: "                   << timestamp << std::endl;
        ss << std::left << std::setw(40) << "device_esn: "                  << device_esn << std::endl;
        ss << std::left << std::setw(40) << "data: "                  << data.to_string() << std::endl;
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_MONITOR
#endif //NCS_MEC_ALARM_MANAGEMENT_DATA_H
