/*********************************************************************************
 * @file		configer.h
 * @brief		configer belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-3-29
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-3-29 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#pragma once

#include "namespace.h"
#include "configer_common.h"
#include "configer_work_param.h"
NAMESPACE_START_OM_COMPONENT_COMMON
using namespace afl::base;
using namespace airos::base::workparam;
#ifndef SUBSCRIBE_NUM
    #define SUBSCRIBE_NUM 1000
#endif

///////////////////////////////////////////////////////////////////////////////////
/////信号机原始数据
struct MonitorMecConfigerSpatSrcData : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/check/spat_src/data_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};
///////////////////////////////////////////////////////////////////////////////////
///感知数据
struct MonitorMecConfigerRsapData : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/check/rsap/data_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};
struct MonitorMecConfigerRsapLink : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/check/rsap/link_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};
///////////////////////////////////////////////////////////////////////////////////
///V2x信控指标数据
struct MonitorMecConfigerCcindexV2xData : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/check/ccindex/tc/data_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};
///////////////////////////////////////////////////////////////////////////////////
///雷达信控指标：动态数据及链路
struct MonitorMecConfigerCcindexTmData : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/check/ccindex/tm/data_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};

///雷达信控指标：动态数据及链路
struct MonitorMecConfigerCcindexTmLink : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/check/ccindex/tm/link_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};
///雷达信控指标：静态数据及链路
struct MonitorMecConfigerCcindexStData : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/check/ccindex/st/data_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};
struct MonitorMecConfigerCcindexStLink : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/check/ccindex/st/link_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};

///毫米波雷达原始数据：数据及链路
struct MonitorMecConfigerCcindexRcp : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/check/rcp_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};
///毫米波雷达原始数据：数据及链路
struct MonitorMecConfigerCcindexRcpData : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/check/rcp/data_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};
struct MonitorMecConfigerCcindexRcpLink : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/check/rcp/link_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};
///////////////////////////////////////////////////////////////////////////////////

struct MonitorMecConfiger : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/monitor/mec";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};
struct SpatSrcDataConfiger : public afl::base::SerializableData
{
    std::string light_esn = "10000";
    std::string channel_readers = "/airos/device/traffic_light/data";
private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(light_esn, "A_light_esn", j, false);
        JsonSerialize(channel_readers, "B_channel_readers", j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(light_esn, "A_light_esn", j, g_flag_empty_novalid);
        JsonDeserialize(channel_readers, "B_channel_readers", j, g_flag_empty_novalid);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "light_esn: " << light_esn << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;
        return ss.str();
    }
};

struct AngleOffsetConfiger : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/usecase";
private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, g_flag_empty_novalid);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};

struct V2xDataConfiger : public afl::base::SerializableData
{
    std::string asn_version = "new_4layer_ext";
    std::string channel_readers_received = "/airos/message/received";
    std::string channel_readers_generated = "/airos/message/generated";
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(asn_version,                     "A_asn_version",                 j, false);
        JsonSerialize(channel_readers_received,                     "B_channel_readers_received",                 j, false);
        JsonSerialize(channel_readers_generated,                     "C_channel_readers_generated",                 j, false);

    }
    
    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(asn_version,                     "A_asn_version",                 j, noUse_isEmptyFlag);
        JsonDeserialize(channel_readers_received,                     "B_channel_readers_received",                 j, noUse_isEmptyFlag);
        JsonDeserialize(channel_readers_generated,                     "C_channel_readers_generated",                 j, noUse_isEmptyFlag);

    }
    
    public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "asn_version: "                   << asn_version                        << std::endl;
        ss << std::left << std::setw(40) << "channel_readers_received: "                   << channel_readers_received                        << std::endl;
        ss << std::left << std::setw(40) << "channel_readers_generated: "                   << channel_readers_generated                        << std::endl;
        return ss.str();
    }
};

//信号灯检测
struct TrafficlightDetectDataConfiger : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/spat/detect/data_";
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(channel_readers,                     "A_channel_readers",                 j, false);

    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(channel_readers,                     "A_channel_readers",                 j, noUse_isEmptyFlag);

    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "channel_readers: "                   << channel_readers                        << std::endl;
        return ss.str();
    }
};
//信号灯数据状态
struct MonitorSpatConfiger : public afl::base::SerializableData
{
    std::string channel_readers = "/v2x/mec/om/spat/status_";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(channel_readers, "A_channel_readers", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(channel_readers, "A_channel_readers", j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40)  << "channel_readers: " << channel_readers << std::endl;

        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_COMMON
