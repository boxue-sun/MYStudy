/*
 * @Author: zhangenwei
 * @Date: 2024-04-24 19:20:01
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-04-24 19:21:11
 * @Description:
 */
#include "parse_info.h"
NAMESPACE_PROTOCOL_THREAD_START

void ParseInfo::printBufferData(afl::net::ByteBuffer& buf, std::string hint)
{
    RSAP_DEBUG_PRINT << "▽▽▽▽▽▽▽▽▽▽▽▽▽▽" <<  hint.c_str() << "- Raw data▽▽▽▽▽▽▽▽▽▽▽▽▽▽";
    std::ostringstream oss;
    for(uint32_t i = 0 ; i < buf.readableBytes(); i++)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint16_t>(*(buf.peek() + i));
        if (i < buf.readableBytes() - 1)
        {
            oss << " ";
        }
    }
    oss << std::endl;
    RSAP_DEBUG_PRINT << oss.str();
    RSAP_DEBUG_PRINT << "△△△△△△△△△△△△△△" << hint.c_str() << "- Raw data△△△△△△△△△△△△△△△";
}

void ParseInfo::parseHearBeat(ByteBuffer* buf)
{
    if(!buf)
    {
        return;
    }
    int offSetLen = sizeof(RsapMsgHeader);
    std::stringstream out;
    out << std::hex << std::setfill(' ');

    RsapMsgHeader &rsapMsgHeader = *((RsapMsgHeader*)buf->peek());
    out << rsapMsgHeaderToString(rsapMsgHeader);
    if(m_CloudServiceConfig.enablePrintParseInfo)
    {
        RSAP_ERROR_PRINT << "[HeartBeat]" << std::endl <<  out.str().c_str();
    }
}

void ParseInfo::parseObjs(ByteBuffer* buf)
{
    if(!buf)
    {
        return;
    }
    int offSetLen = sizeof(RsapMsgHeader);
    std::stringstream out;
    out << std::hex << std::setfill(' ');

    RsapMsgHeader &rsapMsgHeader = *((RsapMsgHeader*)buf->peek());
    out << rsapMsgHeaderToString(rsapMsgHeader);
    out <<  std::endl <<"-------------------------------";
    ObjsDataHead &objsDataHead = *((ObjsDataHead*)(buf->peek() + offSetLen));
    out << objsDataHeadToString(objsDataHead);
    out <<  std::endl <<"-------------------------------";
    offSetLen += sizeof(ObjsDataHead);
    for(int i = 0; i < ntohs(objsDataHead.targetsNum); i++)
    {
        if(ntohs(objsDataHead.targetsNum) > 0)
        {
            //////////////////////////////////////////////////////////////////////
            ObjHead &objHead = *((ObjHead*)(buf->peek() + offSetLen));
            out << objHeadToString(objHead);
            //////////////////////////////////////////////////////////////////////
            //历史目标轨迹
            offSetLen += sizeof(ObjHead);
            ObjHistLoc &objHistLoc = *((ObjHistLoc*)(buf->peek() + offSetLen));
            out << objHistLocToString(objHistLoc, buf, offSetLen);
            //////////////////////////////////////////////////////////////////////
            //目标预测轨迹
            offSetLen += sizeof(ObjHistLoc);
            offSetLen += objHistLoc.histLocNum;

            ObjPredLoc &objPredLoc = *((ObjPredLoc*)(buf->peek() + offSetLen));
            out << objPredLocToString(objPredLoc);
            //////////////////////////////////////////////////////////////////////
            //目标所在车道编号
            offSetLen += sizeof(ObjPredLoc);
            offSetLen += objPredLoc.predLocNum;
            ObjLaneId &objLaneId = *((ObjLaneId*)(buf->peek() + offSetLen));
            out << objLaneIdToString(objLaneId);
            //////////////////////////////////////////////////////////////////////
            //车牌号
            offSetLen += sizeof(ObjLaneId);
            ObjPlateNum &objPlateNum = *((ObjPlateNum*)(buf->peek() + offSetLen));
            out << objPlateNoToString(objPlateNum, buf, offSetLen);

            //////////////////////////////////////////////////////////////////////
            offSetLen += sizeof(ObjPlateNum);
            offSetLen += objPlateNum.lenplateNum;
            ObjsTail &objsTail = *((ObjsTail*)(buf->peek() + offSetLen));
            out << objsTailToString(objsTail);
            offSetLen += sizeof(ObjsTail);

            //打印设备sn的hash数值
            ObjDeviceNum &objDeviceNum = *((ObjDeviceNum*)(buf->peek() + offSetLen - sizeof(ObjDeviceNum)));
            out << objDeviceNoHashToString(objDeviceNum, buf, offSetLen);
            // 输出结果
            offSetLen += objDeviceNum.deviceNum;
        }
        out <<  std::endl <<"-----------------------";
    }

    if(m_CloudServiceConfig.enablePrintParseInfo)
    {
        RSAP_ERROR_PRINT << "[OBJ]" <<  std::endl << out.str().c_str();
    }
}
std::string ParseInfo::objDeviceNoHashToString(ObjDeviceNum &objDeviceNum, ByteBuffer* buf, int offSetLen) const
{

    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        if(objDeviceNum.deviceNum > 0)
        {
//            offSetLen += sizeof(ObjDeviceNum);
            char* plateNo = reinterpret_cast<char*>((char*)(buf->peek() + offSetLen));
            std::string plateNoStr(plateNo, objDeviceNum.deviceNum);
            ss << std::endl <<  std::setfill(' ')  << std::setw(25) << "device-no-hash: " << plateNoStr ;
        }
    }
    catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}

void ParseInfo::parseEvent(ByteBuffer* buf)
{
    if(!buf)
    {
        return;
    }
    int offSetLen = sizeof(RsapMsgHeader);
    std::stringstream out;
    out << std::hex << std::setfill(' ');

    RsapMsgHeader &rsapMsgHeader = *((RsapMsgHeader*)buf->peek());
    out << rsapMsgHeaderToString(rsapMsgHeader);
    out <<  std::endl <<"-------------------------------";

    EventDataHead &eventDataHead = *((EventDataHead*)(buf->peek() + offSetLen));
    out << eventHeadToString(eventDataHead);
    //////////////////////////////////////////////////////////////////////
    offSetLen += sizeof(EventDataHead);
    EventExts &eventExts = *((EventExts*)(buf->peek() + offSetLen));
    out << eventExtsToString(eventExts, buf, offSetLen);
    //////////////////////////////////////////////////////////////////////
    offSetLen += sizeof(EventExts);
    EventDataTail &eventTargetIds= *((EventDataTail*)(buf->peek() + offSetLen));
    out << eventDataTailToString(eventTargetIds, buf, offSetLen);

    if(m_CloudServiceConfig.enablePrintInfoLog)
    {
        RSAP_ERROR_PRINT << "[Event]" << std::endl << out.str().c_str();
    }

}

void ParseInfo::parseStatus(ByteBuffer* buf)
{
    if(!buf)
    {
        return;
    }
    int offSetLen = sizeof(RsapMsgHeader);
    std::stringstream out;
    out << std::hex << std::setfill(' ');

    RsapMsgHeader &rsapMsgHeader = *((RsapMsgHeader*)buf->peek());
    out << rsapMsgHeaderToString(rsapMsgHeader);
    out <<  std::endl <<"-------------------------------";

    StatusDataHead &statusDataHead = *((StatusDataHead*)(buf->peek() + offSetLen));
    out << statusDataHeadToString(statusDataHead);
    //////////////////////////////////////////////////////
    offSetLen += sizeof(StatusDataHead);
    CamStatusHead &camStatusHead = *((CamStatusHead*)(buf->peek() + offSetLen));
    out << camStatusToString(camStatusHead, buf, offSetLen);
    //////////////////////////////////////////////////////
    offSetLen += sizeof(CamStatusHead);
    offSetLen += camStatusHead.camNum * sizeof(CamStatusTail);
    RadarStatusHead &radarStatusHead = *((RadarStatusHead*)(buf->peek() + offSetLen));
    out << radarStatusToString(radarStatusHead, buf, offSetLen);
    //////////////////////////////////////////////////////
    offSetLen += sizeof(RadarStatusHead);
    offSetLen += radarStatusHead.radarNum * sizeof(RadarStatusTail);
    LidarStatusHead &lidarStatusHead = *((LidarStatusHead*)(buf->peek() + offSetLen));
    out << lidarStatusToString(lidarStatusHead, buf, offSetLen);
    if(m_CloudServiceConfig.enablePrintInfoLog)
    {
        RSAP_DEBUG_PRINT << "[Status]" << std::endl << out.str().c_str();
    }
}

std::string ParseInfo::lidarStatusToString(LidarStatusHead &lidarStatusHead, ByteBuffer* buf, int offSetLen) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        ss << std::endl << std::setw(25) << "lidarNum: " << std::dec << static_cast<int>(lidarStatusHead.lidarNum);
        int offSetLenTemp = offSetLen;
        offSetLenTemp += sizeof(LidarStatusHead);
        if(lidarStatusHead.lidarNum > 0)
        {
            for(int i = 0; i < lidarStatusHead.lidarNum; i++)
            {
                LidarStatusTail &lidarStatusTail = *((LidarStatusTail*)(buf->peek() + offSetLenTemp));
                ss << std::endl << std::setfill(' ')  << std::setw(25) << "id: " << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(lidarStatusTail.id) ;
                ss << std::endl << std::setfill(' ')  << std::setw(25) << "lidarId:";
                for(int i = 0; i < 11; i++)
                {
                    if(i == 0)
                    {
                        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(lidarStatusTail.lidarId[i]);
                    }
                    else
                    {
                        ss << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(lidarStatusTail.lidarId[i]);
                    }

                }

                ss << std::endl << std::setfill(' ')  << std::setw(25) << "lidarStatus: " << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(lidarStatusTail.lidarStatus) ;
                ss << std::endl <<" -------------------------- " << std::endl;
                offSetLenTemp += sizeof(LidarStatusTail);
            }
        }
    }catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }

    return ss.str();
}

std::string ParseInfo::radarStatusToString(RadarStatusHead &radarStatusHead, ByteBuffer* buf, int offSetLen) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        ss << std::endl << std::setw(25) << "radarNum: " << std::dec << static_cast<int>(radarStatusHead.radarNum);
        int offSetLenTemp = offSetLen;
        offSetLenTemp += sizeof(RadarStatusHead);
        if(radarStatusHead.radarNum > 0)
        {
            for(int i = 0; i < radarStatusHead.radarNum; i++)
            {
                RadarStatusTail &radarStatusTail = *((RadarStatusTail*)(buf->peek() + offSetLenTemp));
                ss << std::endl << std::setfill(' ') << std::setw(25) << "id: " << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(radarStatusTail.id) ;
                ss << std::endl  << std::setfill(' ') << std::setw(25) << "radarId:";

                for(int i = 0; i < 11; i++)
                {
                    if(i == 0)
                    {
                        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(radarStatusTail.radarId[i]);
                    }
                    else
                    {
                        ss << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(radarStatusTail.radarId[i]);
                    }

                }

                ss << std::endl << std::setfill(' ')  << std::setw(25) << "radarStatus: " << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(radarStatusTail.radarStatus) ;
                ss << std::endl <<" -------------------------- " << std::endl;
                offSetLenTemp += sizeof(RadarStatusTail);
            }
        }
    }catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}


std::string ParseInfo::camStatusToString(CamStatusHead &camStatusHead, ByteBuffer* buf, int offSetLen) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::endl <<" -------------------------- " << std::endl;
        ss << std::hex << std::setfill(' ');
        ss << std::endl << std::setw(25) << "camNum: " << std::dec << static_cast<int>(camStatusHead.camNum);
        int offSetLenTemp = offSetLen;
        offSetLenTemp += sizeof(CamStatusHead);
        if(camStatusHead.camNum > 0)
        {
            for(int i = 0; i < camStatusHead.camNum; i++)
            {
                CamStatusTail &camStatusTail = *((CamStatusTail*)(buf->peek() + offSetLenTemp));
                ss << std::endl << std::setfill(' ')  << std::setw(25) << "id: "  << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(camStatusTail.id) ;
                ss << std::endl << std::setfill(' ')  << std::setw(25) << "camId:" ;

                for(int i = 0; i < 11; i++)
                {
                    if(i == 0)
                    {
                        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(camStatusTail.camId[i]);
                    }
                    else
                    {
                        ss << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(camStatusTail.camId[i]);
                    }

                }
                ss << std::endl << std::setfill(' ')  << std::setw(25) << "camStatus: " << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(camStatusTail.camStatus) ;
                ss << std::endl <<" -------------------------- " << std::endl;
                offSetLenTemp += sizeof(CamStatusTail);
            }
        }
    }catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}


std::string ParseInfo::statusDataHeadToString(StatusDataHead &statusDataHead) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');

        ss << std::endl << std::setfill(' ')  << std::setw(25) << "channelId: " ;
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(statusDataHead.channelId) ;
        ss << "(" << std::dec << static_cast<int>(statusDataHead.channelId) << ")" ;
        ss << std::endl << std::setfill(' ')  << std::setw(25) << "rcuId:" ;
        for(int i = 0; i < 8; i++)
        {
            if(i == 0)
            {
                ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(statusDataHead.rcuId[i]);
            }
            else
            {
                ss << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(statusDataHead.rcuId[i]);
            }

        }
        ss << std::endl << std::setfill(' ')  << std::setw(25) << "status: " ;
        ss << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(statusDataHead.status);
        ss << "(" << std::dec << static_cast<int>(ntohs(statusDataHead.status)) << ")" ;

    }catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}

std::string ParseInfo::eventDataTailToString(EventDataTail &eventDataTail, ByteBuffer* buf, int offSetLen) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        ss << std::endl << std::setfill(' ') << std::setw(25) << "targetIdsLen: " << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(eventDataTail.targetIdsLen );
        if(eventDataTail.targetIdsLen  > 0)
        {
            offSetLen += sizeof(EventDataTail);
            char* targetIds = reinterpret_cast<char*>((char*)(buf->peek() + offSetLen));
            for(int i  = 0; i < eventDataTail.targetIdsLen; i++)
            {
                ss << std::endl << std::setfill(' ') << std::setw(25) << "targetIds-"  << i << ": ";
                for(int j = 0; j < 16; j++)
                {
                    if(j == 0)
                    {
                        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(targetIds[i * 16 + j]);
                    }
                    else
                    {
                        ss << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(targetIds[i * 16 + j]);
                    }
                }
            }

        }
    }catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}

std::string ParseInfo::eventExtsToString(EventExts &eventExts, ByteBuffer* buf, int offSetLen) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        ss << std::endl << std::setw(25) << "extLen: " << std::hex << std::setfill(' ') << std::setw(2) << static_cast<int>(eventExts.extsLen);
        if(eventExts.extsLen > 0)
        {
            offSetLen += sizeof(EventExts);
            char* exts  = reinterpret_cast<char*>((char*)(buf->peek() + offSetLen));
            std::string extsStr(exts, eventExts.extsLen);
            ss << std::endl << std::setw(25) << "exts: " << extsStr ;
        }
    }catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}



std::string ParseInfo::eventHeadToString(EventDataHead &eventHead) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "channelId: ";
        ss << std::hex  << std::setfill('0') <<  std::setw(2) << static_cast<int>(eventHead.channelId) ;
        ss  <<  "(" <<  std::dec << static_cast<int>(eventHead.channelId) << ")" ;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ')  << std::setw(25) << "rcuId:" ;
        std::stringstream ssRcuid;
        for(int i = 0; i < 8; i++)
        {
            if(i == 0)
            {
                ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(eventHead.rcuId[i]);
            }
            else
            {
                ss << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(eventHead.rcuId[i]);
            }

        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "eventType: " ;
        ss << std::hex  << std::setfill('0') << std::setw(2)<< static_cast<int>(eventHead.eventType);
        ss  <<  "(" <<  std::dec << static_cast<int>(eventHead.eventType) << ")" ;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "confidence: ";
        ss << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(eventHead.confidence);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "longitude: "  ;
        ss << std::hex  << std::setfill('0') << std::setw(4)  << static_cast<unsigned long long>(eventHead.longitude);
        ss  <<  "(" <<  std::dec << static_cast<unsigned long long>(ntohl(eventHead.longitude)) << ")" ;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "latitude: ";
        ss << std::hex  << std::setfill('0') << std::setw(2) << static_cast<unsigned long long>(eventHead.latitude);
        ss  <<  "(" <<  std::dec << static_cast<unsigned long long>(ntohl(eventHead.latitude)) << ")" ;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "gnssType: ";
        ss << std::hex  << std::setfill('0') << std::setw(2) << static_cast<unsigned long long>(eventHead.gnssType);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "timestamp: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << boost::endian::big_to_native(static_cast<unsigned long long>(eventHead.timestamp));
        ss  <<  "(" <<  std::dec << boost::endian::big_to_native(static_cast<unsigned long long>(eventHead.timestamp)) << ")" ;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "eventId:";

        for(int i = 0; i < 16; i++)
        {
            if(i == 0)
            {
                ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(eventHead.eventId[i]);
            }
            else
            {
                ss << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(eventHead.eventId[i]);
            }

        }
    }
    catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}

std::string ParseInfo::objsTailToString(ObjsTail &obj) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        ss << std::endl << std::setfill(' ') << std::setw(25) << "plateType: " << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(obj.plateType);
        ss << std::endl << std::setfill(' ') << std::setw(25) << "plateColor: " << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(obj.plateColor );
        ss << std::endl << std::setfill(' ') << std::setw(25) << "objColor: " << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(obj.objColor);
        ss << std::endl << std::setfill(' ') << std::setw(25) << "funtionTimestamp: ";
        ss << std::hex  << std::setfill('0') << std::setw(2) << boost::endian::big_to_native(static_cast<unsigned long long>(obj.funtionTimestamp));

        ss << transTime2LocalTime(obj.funtionTimestamp);
        ss << std::endl << std::setfill(' ') << std::setw(25) << "deviceNum: " << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(obj.deviceNum);
    }
    catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}

/**
* @func: 打印车牌号
*/
std::string ParseInfo::objPlateNoToString(ObjPlateNum &obj, ByteBuffer* buf, int offSetLen) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        ss << std::endl << std::setfill(' ') << std::setw(25) << "lenplateNo: "<< std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(obj.lenplateNum);
        if(obj.lenplateNum > 0)
        {
            offSetLen += sizeof(ObjPlateNum);
            char* plateNo = reinterpret_cast<char*>((char*)(buf->peek() + offSetLen));
            std::string plateNoStr(plateNo, obj.lenplateNum);
            ss << std::endl <<  std::setfill(' ')  << std::setw(25) << "plateNo: " << plateNoStr ;
        }
    }
    catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}


std::string ParseInfo::objLaneIdToString(ObjLaneId &obj) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        ss << std::endl << std::setfill(' ') << std::setw(25) << "laneId: " << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(obj.laneId);
        ss << std::endl << std::setfill(' ') << std::setw(25) << "filterInfoType: " << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(obj.filterInfoType);
    }catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}

std::string ParseInfo::objPredLocToString(ObjPredLoc &obj) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        ss << std::endl << std::setfill(' ') << std::setw(25) << "predLocNum: " << std::setfill('0') << std::setw(2)  << static_cast<int>(obj.predLocNum);

    }catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}

std::string ParseInfo::objHistLocToString(ObjHistLoc &obj, ByteBuffer* buf, int offSetLen) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        ss << std::endl << std::setw(25) << "histLocNum: " << std::setfill('0') << std::setw(2)  << static_cast<int>(obj.histLocNum);

        if(obj.histLocNum > 0)
        {
            for(int i = 0; i < obj.histLocNum; i++)
            {
                offSetLen += sizeof(ObjHistLoc);
                VehiclePositionInfo &vehiclePositionInfo = *((VehiclePositionInfo*)(buf->peek() + offSetLen));
                ///////////////////////////////////////////////////////////////////////////////////////
                ss << std::endl << std::setfill(' ') << std::setw(25) << "longitude: ";
                ss << std::hex  << std::setfill('0') << std::setw(2)   << static_cast<uint64_t>(vehiclePositionInfo.longitude);
                ss << "(" << std::dec <<  static_cast<uint64_t>(vehiclePositionInfo.longitude) << ")";
                ///////////////////////////////////////////////////////////////////////////////////////
                ss << std::endl << std::setfill(' ') << std::setw(25) << "latitude: ";
                ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<uint64_t>(vehiclePositionInfo.latitude) ;
                ss << "(" << std::dec <<  static_cast<uint64_t>(vehiclePositionInfo.latitude) << ")";
                ///////////////////////////////////////////////////////////////////////////////////////
                ss << std::endl << std::setfill(' ') << std::setw(25) << "posConfidence: ";
                ss << std::hex  << std::setfill('0') << std::setw(2)   << static_cast<int>(vehiclePositionInfo.posConfidence);
                ///////////////////////////////////////////////////////////////////////////////////////
                ss << std::endl << std::setfill(' ') << std::setw(25) << "speed: ";
                ss << std::hex  << std::setfill('0') << std::setw(2)   << static_cast<int>(vehiclePositionInfo.speed);
                ss << "(" << std::dec << static_cast<double>((ntohs(vehiclePositionInfo.speed) * 3.6) /100)<< ")";
                ///////////////////////////////////////////////////////////////////////////////////////

                ss << std::endl << std::setfill(' ') << std::setw(25) << "speedConfidence: ";
                ss << std::hex  << std::setfill('0') << std::setw(2)   << static_cast<int>(vehiclePositionInfo.speedConfidence);
                ///////////////////////////////////////////////////////////////////////////////////////
                ss << std::endl << std::setfill(' ') << std::setw(25) << "heading: ";
                ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<uint64_t>(vehiclePositionInfo.heading);
                ss << "(" << std::dec <<   static_cast<uint64_t>(vehiclePositionInfo.heading)<< ")";
                ///////////////////////////////////////////////////////////////////////////////////////
                ss << std::endl << std::setfill(' ') << std::setw(25) << "headConfidence: ";
                ss << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(vehiclePositionInfo.headConfidence);
                ss << std::endl <<" ---------- " << std::endl;
            }
        }


    }catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}

std::string ParseInfo::objHeadToString(ObjHead &objHead) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::hex << std::setfill(' ');
        ss << std::endl << std::setfill(' ')  << std::setw(25) << "uuid: ";
        for(int i = 0; i < 16; i++)
        {
            if(i == 0)
            {
                ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(objHead.uuid[i]);
            }
            else
            {
                ss << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(objHead.uuid[i]);
            }
        }
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ')  << std::setw(25) << "objId: ";
        ss << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(htons(objHead.objId));
        ss << "(" << std::dec << static_cast<int>(ntohs(objHead.objId)) << ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "type: ";
        ss << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.type);
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "status: ";
        ss << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.status);
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl  << std::setfill(' ') << std::setw(25) << "len: " ;
        ss << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(objHead.len);
        ss << "(" << std::dec << static_cast<int>(ntohs(objHead.len))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl  << std::setfill(' ') << std::setw(25) << "width: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.width);
        ss << "(" << std::dec << static_cast<int>(ntohs(objHead.width))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl  << std::setfill(' ') << std::setw(25) << "height: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.height);
        ss << "(" << std::dec << static_cast<int>(ntohs(objHead.height))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl  << std::setfill(' ') << std::setw(25) << "longitude: ";
        ss << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(objHead.longitude);
        ss << "(" << std::dec << static_cast<int>(ntohl(objHead.longitude))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl  << std::setfill(' ') << std::setw(25) << "latitude: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.latitude);
        ss << "(" << std::dec << static_cast<int>(ntohl(objHead.latitude))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////

        ss << std::endl  << std::setfill(' ') << std::setw(25) << "locEast: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.locEast);
        ss << "(" << std::dec << static_cast<int>(ntohl(objHead.locEast))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////

        ss << std::endl  << std::setfill(' ') << std::setw(25) << "locNorth: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.locNorth);
        ss << "(" << std::dec << static_cast<int>(ntohl(objHead.locNorth))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////

        ss << std::endl  << std::setfill(' ') << std::setw(25) << "posConfidence: " ;
         ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.posConfidence);
        ss << "(" << std::dec << static_cast<int>(objHead.posConfidence)<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl  << std::setfill(' ') << std::setw(25) << "elevation: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.elevation);
        ss << "(" << std::dec << static_cast<int>(ntohl(objHead.elevation))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////

        ss << std::endl << std::setfill(' ') << std::setw(25) << "elevConfidence: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.elevConfidence);
        ss << "(" << std::dec << static_cast<int>(objHead.elevConfidence)<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "speed: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.speed);
        ss << "(" << std::dec << static_cast<double>((ntohs(objHead.speed) * 3.6) /100)<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////

        ss << std::endl << std::setfill(' ') << std::setw(25) << "speedConfidence: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.speedConfidence);
        ss << "(" << std::dec << static_cast<int>(objHead.speedConfidence)<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "speedEast: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.speedEast);
        ss << "(" << std::dec << static_cast<int>(ntohs(objHead.speedEast))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////

        ss << std::endl << std::setfill(' ') << std::setw(25) << "speedEastConfidence: " ;
        ss << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.speedEastConfidence);
        ss << "(" << std::dec << static_cast<int>(objHead.speedEastConfidence)<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "speedNorth: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.speedNorth);
        ss << "(" << std::dec << static_cast<int>(ntohs(objHead.speedNorth))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "speedNorthConfidence: ";
        ss << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.speedNorthConfidence);
        ss << "(" << std::dec << static_cast<int>(objHead.speedNorthConfidence)<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "heading: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.heading);
        ss << "(" << std::dec << static_cast<int>(ntohl(objHead.heading))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "headConfidence: ";
        ss << std::setfill('0') << std::setw(2) << static_cast<int>(objHead.headConfidence);
        ss << "(" << std::dec << static_cast<int>(objHead.headConfidence)<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "accelVert: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.accelVert);
        ss << "(" << std::dec << static_cast<int>(ntohs(objHead.accelVert))<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "accelVertConfidence: ";
        ss << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.accelVertConfidence);
        ss << "(" << std::dec << static_cast<int>(objHead.accelVertConfidence)<< ")";
        ////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "trackedTimes: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objHead.trackedTimes);
        ss << "(" << std::dec << static_cast<int>(ntohl(objHead.trackedTimes))<< ")";
    }catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}

std::string ParseInfo::rsapMsgHeaderToString(RsapMsgHeader &rsapMsgHeader) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        ss << std::setw(25) << "StartFlag: " << std::hex << static_cast<int>(rsapMsgHeader.StartFlag) ;
        int dataLen = (static_cast<int>(rsapMsgHeader.DataLen[0]) << 16) |
                      (static_cast<int>(rsapMsgHeader.DataLen[1]) << 8) |
                      static_cast<int>(rsapMsgHeader.DataLen[2]);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setw(25) << "DataLen: "
           << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(rsapMsgHeader.DataLen[0])
           << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(rsapMsgHeader.DataLen[1])
           << " " << std::hex << std::setfill('0') << std::setw(2) <<  static_cast<int>(rsapMsgHeader.DataLen[2])
           << "(" << std::dec  << dataLen << ")";
        ss << std::hex << std::setfill(' ');
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setw(25) << "DataType: " << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(rsapMsgHeader.DataType);
        ss  <<  "(" <<  std::dec << static_cast<int>(rsapMsgHeader.DataType) << ")" ;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "Version: " ;
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(rsapMsgHeader.Version);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "TimestampMS: "  ;
        ss << std::hex  << std::setfill(' ') << std::setw(2) << static_cast<unsigned long long>(rsapMsgHeader.TimestampMS);
        ss  <<  "(" <<  std::dec << static_cast<unsigned long long>(ntohs(rsapMsgHeader.TimestampMS))<< ")" ;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl <<  std::hex << std::setfill(' ') << std::setw(25) << "TimestampMIN: "  ;
        ss << std::hex  << std::setfill(' ') << std::setw(2)  << static_cast<unsigned long long>(rsapMsgHeader.TimestampMIN);
        ss  <<  "(" <<  std::dec << static_cast<unsigned long long>(ntohl(rsapMsgHeader.TimestampMIN) )<< ")" ;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        time_t  minTemp = static_cast<unsigned long long>(ntohl(rsapMsgHeader.TimestampMIN));
        uint64_t millisecondsTemp = static_cast<unsigned long long>(ntohs(rsapMsgHeader.TimestampMS));
        uint64_t timeStampSum = minTemp * (60000) + millisecondsTemp;
        if(m_CloudServiceConfig.enableUseAddUtc8)
        {
            ss << std::endl << std::dec <<  std::setfill(' ') << std::setw(25) << "utc-0:" << std::dec <<  std::setfill('0')  <<   timeStampSum - UTC8_MILLSECODND_DIFF_VALUE;
        }
        else
        {
            ss << std::endl << std::dec <<  std::setfill(' ') << std::setw(25) << "utc-0:" << std::dec <<  std::setfill('0')  <<   timeStampSum;
        }
        // 将时间戳转换为tm结构
        ss << std::endl << std::dec <<  std::setfill(' ') << std::setw(25) << "time-format:(北京时间)";
       time_t utc8Second =  (timeStampSum)/1000;
        tm* datetime = gmtime((const time_t *)&utc8Second);
        ss  <<  (datetime->tm_year + 1900) << '-'  // tm_year是从1900年开始的年数
       << std::setw(2) << std::dec <<  std::setfill('0')  << (datetime->tm_mon + 1) << '-'  // tm_mon是从0开始的月份数
       << std::setw(2) << std::dec <<  std::setfill('0')  << datetime->tm_mday << ' '  // tm_mday是月份中的第几天
       << std::setw(2) << std::dec <<  std::setfill('0')  << datetime->tm_hour + 8<< ':'  // tm_hour是24小时制的小时数
       << std::setw(2) << std::dec <<  std::setfill('0')  << datetime->tm_min << ':'  // tm_min是小时中的分钟数
       << std::setw(2) << std::dec <<  std::setfill('0')  << datetime->tm_sec  << ':'  // tm_sec是分钟中的秒数
       << std::setw(2) << std::dec <<  std::setfill('0')  << rsapMsgHeader.TimestampMS;

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }
    catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();

}
std::string ParseInfo::objsDataHeadToString(ObjsDataHead &objsDataHead) const
{
    std::stringstream ss;
    ss << std::endl;
    try{
        ss << std::hex << std::setfill(' ');

        ss << std::endl << std::hex  << std::setfill(' ') << std::setw(25) << "channelId: " ;
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objsDataHead.channelId) ;
        ss << "(" << std::dec <<  static_cast<int>(objsDataHead.channelId) << ")";
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "rcuId: ";
        for(int i = 0; i < 8; i++)
        {
            if(i == 0)
            {
                ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(objsDataHead.rcuId[i]);
            }
            else
            {
                ss << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(objsDataHead.rcuId[i]);
            }

        }

//        ss  <<  "(" <<  std::string(reinterpret_cast<const char*>(objsDataHead.rcuId), 8) << ")" ;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "deviceType:" ;
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(objsDataHead.deviceType);
        ss  <<  "(" <<  std::dec <<  static_cast<int>(objsDataHead.deviceType) << ")" ;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "deviceId:";
        for(int i = 0; i < 11; i++)
        {
            if(i == 0)
            {
                ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(objsDataHead.deviceId[i]);
            }
            else
            {
                ss << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(objsDataHead.deviceId[i]);
            }
        }
//        ss  <<  "(" <<  std::string(reinterpret_cast<const char*>(objsDataHead.deviceId),  11) << ")" ;
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "timestampOfDevOut: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << boost::endian::big_to_native(static_cast<unsigned long long>(objsDataHead.timestampOfDevOut));

        ss << transTime2LocalTime(objsDataHead.timestampOfDevOut);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "timestampOfDetIn: " ;
        ss << std::hex  << std::setfill('0') << std::setw(2) << boost::endian::big_to_native(static_cast<unsigned long long>(objsDataHead.timestampOfDetIn));

        ss << transTime2LocalTime(objsDataHead.timestampOfDetIn);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "timestampOfDetOut: ";
        ss << std::hex  << std::setfill('0') << std::setw(2) << boost::endian::big_to_native(static_cast<unsigned long long>(objsDataHead.timestampOfDetOut));

        ss << transTime2LocalTime(objsDataHead.timestampOfDetOut);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ') << std::setw(25) << "gnssType: "            << std::hex  << std::setfill('0') << std::setw(2) << static_cast<int>(objsDataHead.gnssType);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ss << std::endl << std::setfill(' ')  << std::setw(25) << "targetsNum: ";
        ss << std::hex  << std::setfill('0') << std::setw(2)  << static_cast<int>(ntohs(objsDataHead.targetsNum));
        ss << "(" << std::dec  << static_cast<int>(ntohs(objsDataHead.targetsNum)) << ")" ;

    }
    catch(afl::util::Exception & e)
    {
        RSAP_ERROR_PRINT << "[error]" << e.what();
    }
    return ss.str();
}
std::string ParseInfo::deviceCodeTransInverse(std::string byteDataStr)
{
    std::string codeStr;
    for (unsigned char byte : byteDataStr)
    {
        // 将每个字节转换为两位十六进制数
        int byteValue = static_cast<int>(byte);
        std::string byteStr = std::to_string(byteValue);
        // 如果需要，可以在前面补零以确保两位数
        while (byteStr.length() < 2)
        {
            byteStr = "0" + byteStr;
        }
        codeStr += byteStr;
    }
    return codeStr;
}

std::string ParseInfo::transTime2LocalTime(const uint64_t&  timestamp) const
{
    std::stringstream ss;
    ss << std::endl;
    time_t utc8Second =  boost::endian::big_to_native(timestamp)/1000;
    auto utc_time = std::chrono::system_clock::from_time_t(utc8Second + UTC8_SECODND_DIFF_VALUE);
    auto beijing_time = utc_time + std::chrono::hours(8);
    auto beijing_time_t = std::chrono::system_clock::to_time_t(beijing_time);

    tm beijing_tm;
    gmtime_r(&beijing_time_t, &beijing_tm); // convert time_t to tm

    ss << "(北京时间:" << std::dec << beijing_tm.tm_year + 1900 << '-'
       << std::setw(2) << std::dec << std::setfill('0') << beijing_tm.tm_mon + 1 << '-'
       << std::setw(2) << std::dec << std::setfill('0') << beijing_tm.tm_mday << ' '
       << std::setw(2) << std::dec << std::setfill('0') << beijing_tm.tm_hour << ':'
       << std::setw(2) << std::dec << std::setfill('0') << beijing_tm.tm_min << ':'
       << std::setw(2) << std::dec << std::setfill('0') << beijing_tm.tm_sec << ':'
       << std::setw(2) << std::dec << std::setfill('0') << beijing_tm.tm_sec % 1000 << ')';
    return ss.str();
}


NAMESPACE_PROTOCOL_THREAD_END
