/*********************************************************************************
 * @file		v2x_codec_component.h
 * @brief		v2x_codec_component belongs to CICTCI
 * @details		
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-3-17
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-3-17 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#pragma once
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <thread>
#include <mutex>
#include <future>
#include <dirent.h>
#include <regex.h>
#include <sys/statfs.h>
#include <limits.h>
#include <memory>
#include <string>
#include <jsoncpp/json/json.h>
#include "base/common/network/print.h"
#include "data_model/namespace.h"
#include "data_model/protocol_data.h"
#include "data_model/objs_data.h"
#include "data_model/event_data.h"
#include "data_model/status_data.h"
#include "data_model/tcp_server_config_data.h"
#include "base/common/log.h"
#include "base/common/network/byte_buffer.h"
#include "base/common/network/exception.h"
#include <boost/endian/conversion.hpp>
#include <chrono>
#include <ctime>
NAMESPACE_PROTOCOL_THREAD_START

using namespace os::v2x::protocol;
using namespace afl::net;
class ParseInfo
{
 public:
    ParseInfo() = default;

    ~ParseInfo() = default;
    void printBufferData(afl::net::ByteBuffer& buf, std::string hint);
    void parseHearBeat(ByteBuffer* buf);
    void parseObjs(ByteBuffer* buf);
    void parseEvent(ByteBuffer* buf);
    void parseStatus(ByteBuffer* buf);
    std::string objDeviceNoHashToString(ObjDeviceNum &objDeviceNum, ByteBuffer* buf, int offSetLen) const;
    std::string eventHeadToString(EventDataHead &eventHead) const;
    std::string eventExtsToString(EventExts &eventExts, ByteBuffer* buf, int offSetLen) const;
    std::string eventTargetIdsToString(EventExts &eventExts, ByteBuffer* buf, int offSetLen) const;
    std::string eventDataTailToString(EventDataTail &eventDataTail, ByteBuffer* buf, int offSetLen) const;

    std::string statusDataHeadToString(StatusDataHead &statusDataHead) const;
    std::string camStatusToString(CamStatusHead &camStatusHead, ByteBuffer* buf, int offSetLen) const;
    std::string radarStatusToString(RadarStatusHead &radarStatusHead, ByteBuffer* buf, int offSetLen) const;
    std::string lidarStatusToString(LidarStatusHead &lidarStatusHead, ByteBuffer* buf, int offSetLen) const;
    std::string rsapMsgHeaderToString(RsapMsgHeader &rsapMsgHeader) const;
    std::string objsDataHeadToString(ObjsDataHead &objsDataHead) const;
    std::string objHeadToString(ObjHead &objHead) const;
    std::string objHistLocToString(ObjHistLoc &obj, ByteBuffer* buf, int offSetLen) const;
    std::string objPredLocToString(ObjPredLoc &obj) const;
    std::string objLaneIdToString(ObjLaneId &obj) const;
    std::string objPlateNoToString(ObjPlateNum &obj, ByteBuffer* buf, int offSetLen) const;
    std::string objsTailToString(ObjsTail &obj) const;
    std::string deviceCodeTransInverse(std::string byteDataStr);

    void setCloudServiceConfig(CloudServiceConfig temp)
    {
        m_CloudServiceConfig = temp;
    }
    std::string transTime2LocalTime(const uint64_t&  timestamp) const;
private:
    CloudServiceConfig                      m_CloudServiceConfig;
};

NAMESPACE_PROTOCOL_THREAD_END
