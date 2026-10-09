/*
 * @Author: zhangenwei
 * @Date: 2024-01-19 10:20:24
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-01-19 10:20:24
 * @Description:
 * @FilePath: /airos/base/device_connect/mec/cictci/cictci_mec.cc
 */

#include "cictci_mec.h"

#include <GeographicLib/UTMUPS.hpp>

#include "base/common/log.h"
#include "base/common/network/time_stamp.h"
namespace os {
namespace v2x {
namespace device {
CICTCIMec::~CICTCIMec()
{
    this->Stop();
    if(timer_monitor_sensor_data_in > 0)
    {
        m_Eventloop->cancelTimer(timer_monitor_sensor_data_in);
    }
}

bool CICTCIMec::Init(const std::string &config_file)
{
    if(!has_send_once)
    {
        sender_(output_data_obj_);
        has_send_once = true;
    }

    try {
        YAML::Node root_node = YAML::LoadFile(config_file);
        if (!root_node.IsMap())
        {
            MEC_SERVICE_LOG_ERROR << "config file format invalid, file=" << config_file;
            return false;
        }

        std::string ip = root_node["ip"].as<std::string>();
        uint16_t port = root_node["port"].as<int>();
        uint16_t local_port = root_node["local_port"].as<int>();
        sensor_data_in_port = local_port;
        std::string local_ip;
        if (root_node["local_ip"].IsScalar())
        {
            local_ip = root_node["local_ip"].as<std::string>();
        }
        std::string protocol = "udp";
        if (root_node["protocol"].IsScalar())
        {
            protocol = root_node["protocol"].as<std::string>();
        }

        MEC_SERVICE_LOG_WARN << "[remote ip]: " << ip << "[remote port]" << port
                     << "[host ip]" << local_ip << "[host port]" << local_port
                     << "[protocol]" << protocol;

        if (!InitProtocol(ip, port, local_ip, local_port, protocol))
        {
            MEC_SERVICE_LOG_ERROR << "cictci-mec worker init failed";
            return false;
        }
    } catch (...) {
        MEC_SERVICE_LOG_ERROR << "Config file format wrong! Please check the format(e.g. "
                      "indentation), file="
                   << config_file;
        return false;
    }
    m_Task.reset(new std::thread([&](){ InitEventLoop(); }));

    return true;
}
bool CICTCIMec::InitEventLoop()
{
    m_Eventloop = std::make_shared<afl::net::EventLoop>();
    if(m_Eventloop)
    {
        timer_monitor_sensor_data_in = m_Eventloop->addTimer(std::bind(&CICTCIMec::MonitorSensorDataIn, this), 10, true);
    }
    m_Eventloop->loop();
    return true;
}
void CICTCIMec::MonitorSensorDataIn()
{
    if(receive_obj_package_old == receive_obj_package_new && receive_obj_package_old != 0)
    {
        MEC_SERVICE_LOG_ERROR << "[notice] receive data";
        receive_obj_package_old++;
    }
    else
    {
        output_monitor_->Clear();
        auto* mec_sensor_data_monitor =  output_monitor_->mutable_mec_sensor_data_monitor();
        mec_sensor_data_monitor->set_tag(airos::usecase::MonitorMsgTag::MONITOR_TAG_SENSOR_DATA_IN);
        mec_sensor_data_monitor->set_timestamp( afl::util::TimeStamp::now(true).millSeconds());

        mec_sensor_data_monitor->set_sensor_data_in_port(std::to_string(sensor_data_in_port));
        mec_sensor_data_monitor->set_sensor_data_in_flag(false);
        mec_sensor_data_monitor->set_sensor_data_in_obj_package(0);
        sender_(output_monitor_);
        MEC_SERVICE_LOG_ERROR << "[notice] no sensor-data!" << output_monitor_->ShortDebugString();
        receive_obj_package_old++;
    }
}

void CICTCIMec::Start()
{
    thread_process_recv_.reset(new std::thread(std::bind(&CICTCIMec::TaskProcessRecvFrame, this)));
    return;
}

void CICTCIMec::WriteToDevice(const std::shared_ptr<const airos::usecase::EventOutputResult> &re_proto)
{
    return;
}

bool CICTCIMec::InitProtocol(const std::string &remote_ip, const uint16_t remote_port, const std::string &host_ip,
                           const uint16_t host_port, const std::string &protocol)
{
    CictciCommunication::ProtocolType protocol_type;
    if (protocol == "tcp")
    {
        protocol_type = CictciCommunication::ProtocolType::TCP;
    } else if (protocol == "udp")
    {
        protocol_type = CictciCommunication::ProtocolType::UDP;
    } else
    {
        MEC_SERVICE_LOG_ERROR << "not support this protocol: " << protocol;
        return false;
    }

    communication_.reset(new CictciCommunication());
    if (!communication_->Init(remote_ip, remote_port, host_ip, host_port, protocol_type))
    {
        MEC_SERVICE_LOG_ERROR << "GatCommunication init error";
        return false;
    }
    return true;
}

ssize_t CICTCIMec::SendFrame(uint8_t *packet_addr, size_t packet_len)
{
    return communication_->SendData(packet_addr, packet_len);
}

void CICTCIMec::TaskProcessRecvFrame()
{    
    auto oriBuf = afl::net::ByteBuffer(512, 65536);
    ssize_t recv_len = -1;
    while (!stop_)
    {
        recv_len = communication_->RecvData(oriBuf);
        if (recv_len > 0)
        {
            afl::base::json j;
            try
            {
                j = afl::base::json::parse(oriBuf.peek(), oriBuf.peek() + oriBuf.readableBytes());
            }
            catch (...)
            {
                oriBuf.retrieveAll();
                MEC_SERVICE_LOG_ERROR << "Get msg Json parse error!";
                MEC_IN_DEBUG_PRINT << "Get msg Json parse error!";
                continue;
            }

             MEC_SERVICE_LOG_INFO << "[mec-in]" << j.dump().c_str();

            if (j.find("MsgType") == j.end()) {
              MEC_SERVICE_LOG_ERROR << "Can not find MsgType!!!";
              MEC_IN_DEBUG_PRINT  << "Can not find MsgType!!!";
              return;
            }

            int type = j["MsgType"];
            MEC_SERVICE_LOG_INFO << "Recv msg MsgType: " << type;
            switch (type)
            {
            case 2011:
                ParseJsonData2PbDataObj(j);
                break;
            case 2012:
                ParseJsonData2PbDataEvent(j);
                break;
//            case 5001:
//                ParseJsonData2PbDataHeartBeat(j);
//                break;
            case 2016:
                ParseJsonData2PbDataAngleOffset(j);
                break;
            case 2018:
                ParseJsonData2PbDataTrafficlight(j);
                break;
            case 2101:
                ParseJsonData2PbDataTrafficFlow(j);
                break;
            default:
                break;
            }
            
            oriBuf.retrieveAll();
        }
    }

    MEC_SERVICE_LOG_INFO << "task over : process response.";
}

bool CICTCIMec::ParseJsonData2PbDataObj(json& j)
{ 
    cictci_object cictciObject;
    try
    {
        cictciObject  = j;
    }
    catch(json::exception &e)
    {
        MEC_SERVICE_LOG_ERROR << "json par error " << e.what();
        MEC_IN_DEBUG_PRINT  << "json par error " << e.what();
        return false;
    }

    //  MEC_SERVICE_LOG_INFO << j.dump().c_str();
    output_data_obj_->Clear();
    auto header = output_data_obj_->mutable_header();
    try{
        ///////////////////////////////////////////////
        auto* mec_sensor_data_monitor =  output_monitor_->mutable_mec_sensor_data_monitor();
        mec_sensor_data_monitor->set_tag(airos::usecase::MonitorMsgTag::MONITOR_TAG_SENSOR_DATA_IN);
        mec_sensor_data_monitor->set_timestamp( afl::util::TimeStamp::now(true).millSeconds());
        mec_sensor_data_monitor->set_sensor_data_in_port(std::to_string(sensor_data_in_port));

        // Set the header fields.
        header->set_camera_timestamp(TimeStampTransfer(cictciObject.m_Timestamp) * 1000);  // ns
        header->set_frame_id(cictciObject.m_DevNo);
        header->set_timestamp_sec(airos::base::TimeUtil::GetCurrentTime());
        header->set_sequence_num(++mec_seqnum_);
        //////////////////////////
        //add zhangenwei
        MEC_SERVICE_LOG_INFO << "[m_Timestamp-json]" <<cictciObject.m_Timestamp << "[m_Timestamp-trans]" << (TimeStampTransfer(cictciObject.m_Timestamp));
        header->set_timestamp_millisecond(TimeStampTransfer(cictciObject.m_Timestamp));
        header->set_dev_no(cictciObject.m_DevNo);
        if(cictciObject.m_RadarNo_Empty)
        {
            header->set_radar_no(cictciObject.m_RadarNo);
        }
        if(cictciObject.m_LidarNo_Empty)
        {
            header->set_lidar_no(cictciObject.m_LidarNo);
        }
        header->set_mec_no(cictciObject.m_MecNo);
        header->set_total_package_num(mec_seqnum_);
        bool sensor_data_in_flag_temp = true;
        mec_sensor_data_monitor->set_sensor_data_in_flag(sensor_data_in_flag_temp);
        mec_sensor_data_monitor->set_sensor_data_in_obj_package(mec_seqnum_);
        receive_obj_package_new = mec_seqnum_;
        receive_obj_package_old = mec_seqnum_;
		//删除4以上的ptcType用于现场测试验证
        uint32_t needDelPtc = 0;
        for (size_t i = 0; i < cictciObject.m_Obj_List.size(); ++i)
        {
            auto cictciObjectDetail = cictciObject.m_Obj_List[i];
            if( (cictciObjectDetail.m_PtcType == 0 ) || (cictciObjectDetail.m_PtcType >= 4) )
            {
                MEC_SERVICE_LOG_INFO << "[notice]PTC type is not vehicle or pedestrian bicycle, contine process next ptc!" ;
                needDelPtc++;
            }
        }
        ////////////////////////
        mec_recv_objnum_ += (cictciObject.m_Obj_List.size() - needDelPtc);
        header->set_total_obj_num(mec_recv_objnum_);
        mec_sensor_data_monitor->set_sensor_data_in_obj_num(mec_recv_objnum_);
        MEC_SERVICE_LOG_WARN << "[monitor-out]" << output_monitor_->ShortDebugString();
        sender_(output_monitor_);
        // Add some obstacles to the message.
    	MEC_SERVICE_LOG_INFO << "size:" << (cictciObject.m_Obj_List.size() - needDelPtc);
        for (size_t i = 0; i < cictciObject.m_Obj_List.size(); ++i)
        {
            auto cictciObjectDetail = cictciObject.m_Obj_List[i];
            if( (cictciObjectDetail.m_PtcType == 0 ) || (cictciObjectDetail.m_PtcType >= 4))
            {
                MEC_SERVICE_LOG_INFO << "[notice]PTC type is not vehicle or pedestrian bicycle, contine process next ptc!" ;
                continue;
            }
            std::stringstream ss_check;
            ss_check << "[sensor-in]" << TimeStampTransfer(cictciObject.m_Timestamp) << ",";
            airos::perception::PerceptionObstacle *obstacle = output_data_obj_->add_perception_obstacle();

        
            // Set the obstacle fields.
            string sensorReportId = "jf_" + cictciObjectDetail.m_ID;
            obstacle->set_id_str(cictciObjectDetail.m_ID);
            ss_check << cictciObjectDetail.m_ID << ",";
            obstacle->set_id(SENSOROBJIDALLOCATOR_INSTANCE_REF.getObjId(sensorReportId));
            obstacle->set_length(cictciObjectDetail.m_VehL);
            if (!cictciObjectDetail.m_VehW_Empty)
            {
                obstacle->set_width(cictciObjectDetail.m_VehW);
            }
    //        else
    //        {
    //            obstacle->set_width(1.8);
    //        }

            if (!cictciObjectDetail.m_VehH_Empty)
            {
                obstacle->set_height(cictciObjectDetail.m_VehH);
            }
    //        else
    //        {
    //            obstacle->set_height(1.8);
    //        }

            // 速度暂时以车头方向为x，后续看是否需要改动
            obstacle->mutable_velocity()->set_x(cictciObjectDetail.m_PtcSpeed / 3.6);

            // 航向角赋值
            obstacle->set_theta(cictciObjectDetail.m_PtcHeading);

            // 经纬度坐标转UTM坐标
            int zone;
            bool dummy;
            double x, y;
            GeographicLib::UTMUPS::Forward(cictciObjectDetail.m_PtcLat, cictciObjectDetail.m_PtcLon, zone, dummy, x, y);

            obstacle->mutable_position()->set_x(x);
            obstacle->mutable_position()->set_y(y);
            obstacle->mutable_position()->set_z(cictciObjectDetail.m_PtcEle);
            obstacle->mutable_position()->set_zone(zone);

            //////////////////////////
            //add zhangenwei
            int type = 0;
            int ptcType = 0;

            switch (cictciObjectDetail.m_PtcType)
            {
                case 0:
                    type = airos::perception::PerceptionObstacle_Type_UNKNOWN;
                    ptcType = airos::perception::MEC_PTC_UNKNOWN;
                    break;
                case 1:
                    type = airos::perception::PerceptionObstacle_Type_VEHICLE;
                    ptcType = airos::perception::MEC_PTC_MOTOR;
                    break;
                case 2:
                    type = airos::perception::PerceptionObstacle_Type_BICYCLE;
                    ptcType = airos::perception::MEC_PTC_NON_MOTOR;
                    break;
                case 3:
                    type = airos::perception::PerceptionObstacle_Type_PEDESTRIAN;
                    ptcType = airos::perception::MEC_PTC_PEDESTRIAN;
                    break;
                case 4:
                    // type = Ptc_rsu;
                    type = airos::perception::PerceptionObstacle_Type_UNKNOWN;
                    ptcType = airos::perception::PerceptionObstacle_Type_UNKNOWN;
                    break;
                case 5:
                    // type = 5;    //obstacle
                    type = airos::perception::PerceptionObstacle_Type_UNKNOWN;
                     ptcType = airos::perception::PerceptionObstacle_Type_UNKNOWN;
                    break;
                default:
                	MEC_SERVICE_LOG_ERROR << "(error) unknown PtcType!" << cictciObjectDetail.m_PtcType;
                    type = airos::perception::PerceptionObstacle_Type_UNKNOWN;
                    ptcType = airos::perception::PerceptionObstacle_Type_UNKNOWN;
                    break;
            }
            obstacle->set_type(static_cast<airos::perception::PerceptionObstacle_Type>(type));
            obstacle->set_ptc_type(static_cast<airos::perception::MecPtcType>(ptcType));

            obstacle->mutable_position_gcs()->set_lon(cictciObjectDetail.m_PtcLon);
            ss_check << cictciObjectDetail.m_PtcLon << ",";
            obstacle->mutable_position_gcs()->set_lat(cictciObjectDetail.m_PtcLat);
            ss_check << cictciObjectDetail.m_PtcLat << ",";
            ss_check << cictciObjectDetail.m_PtcType << ",";
            if (!cictciObjectDetail.m_PtcEle_Empty)
            {
                obstacle->mutable_position_gcs()->set_ele(cictciObjectDetail.m_PtcEle);
            }

            obstacle->set_ptc_speed(cictciObjectDetail.m_PtcSpeed);
            if(!cictciObjectDetail.m_PtcHeading_Empty)
            {
                obstacle->set_ptc_heading(cictciObjectDetail.m_PtcHeading);
            }

            if(!cictciObjectDetail.m_LanneNo_Empty)
            {
                obstacle->set_lane_no(cictciObjectDetail.m_LanneNo);
            }

            if(!cictciObjectDetail.m_VehType_Empty)
            {
                obstacle->set_vehicle_type((airos::perception::MecVehicleType)cictciObjectDetail.m_VehType);
                ss_check << cictciObjectDetail.m_VehType << ",";
            }
            else
            {
                ss_check << 10 << ",";
            }
            ss_check << cictciObjectDetail.m_PtcHeading;
            if(!cictciObjectDetail.m_NonVehType_Empty)
            {
                obstacle->set_non_vehicle_type((airos::perception::MecNonVehicleType)cictciObjectDetail.m_NonVehType);
            }

            if(!cictciObjectDetail.m_Tracking_Empty)
            {
                obstacle->set_tracking(cictciObjectDetail.m_Tracking);
            }

            if (!cictciObjectDetail.m_PlateNum_Empty)
            {
                obstacle->set_plate_num(cictciObjectDetail.m_PlateNum);
            }

            if (!cictciObjectDetail.m_PlateColor_Empty)
            {
                if(cictciObjectDetail.m_PlateColor > airos::perception::MecPlateColorType::PLATE_COLOR_TYPE_GREEN || cictciObjectDetail.m_PlateColor < airos::perception::MecPlateColorType::PLATE_COLOR_TYPE_OTHER)
                {
                    MEC_SERVICE_LOG_ERROR << "[error]plate color value > 5 or value < 0, [value]" << cictciObjectDetail.m_PlateColor;
                }
                else
                {
                    obstacle->set_plate_color((airos::perception::MecPlateColorType)cictciObjectDetail.m_PlateColor);
                }

            }
            if (!cictciObjectDetail.m_ObjColor_Empty)
            {
                if(cictciObjectDetail.m_PlateColor > airos::perception::MecObjColorType::OBJ_COLOR_TYPE_PURPLE || cictciObjectDetail.m_PlateColor < airos::perception::MecObjColorType::OBJ_COLOR_TYPE_OTHER)
                {
                    MEC_SERVICE_LOG_ERROR << "[error]color value > 12 or value < 0, [value]" << cictciObjectDetail.m_PlateColor;
                }
                else
                {
                    obstacle->set_obj_color((airos::perception::MecObjColorType)cictciObjectDetail.m_ObjColor);
                }
            }

            obstacle->set_length(cictciObjectDetail.m_VehL);
            if(!cictciObjectDetail.m_VehW_Empty)
            {
                obstacle->set_width(cictciObjectDetail.m_VehW);
            }

            if(!cictciObjectDetail.m_VehH_Empty)
            {
                obstacle->set_height(cictciObjectDetail.m_VehH);
            }

            // RSAP_DEBUG_PRINT << ss_check.str();
        }
    }catch(afl::util::Exception& e)
    {
        MEC_SERVICE_LOG_ERROR << "json par error " << e.what();
        MEC_IN_DEBUG_PRINT << "json par error " << e.what();
        return false;
    }
    MEC_SERVICE_LOG_INFO << "summary: [Send package total num]" << mec_seqnum_ << "   [total obj num]" << mec_recv_objnum_ << "    [timeStamp]" <<  std::fixed << airos::base::TimeUtil::GetCurrentTime() * 1000;
    MEC_SERVICE_LOG_WARN << "[mec-out]" << output_data_obj_->ShortDebugString();
    sender_(output_data_obj_);
    return true;
}

bool CICTCIMec::ParseJsonData2PbDataEvent(json &j) {
    CictciEvent cictciEvent;
    try
    {
        cictciEvent  = j;
    }
    catch(json::exception &e)
    {
        MEC_SERVICE_LOG_ERROR << "json par error " << e.what();
        MEC_IN_DEBUG_PRINT << "json par error " << e.what();
        return false;
    }
    output_data_event_->Clear();
    auto header = output_data_event_->mutable_header();
    try{
        // Set the header fields.
        header->set_camera_timestamp(TimeStampTransfer(cictciEvent.m_Timestamp) * 1000);  // ns
        header->set_frame_id(cictciEvent.m_DevNo);
        header->set_timestamp_sec(airos::base::TimeUtil::GetCurrentTime());
        header->set_sequence_num(++mec_seqnum_);
        /////////////////////////////////////////////////////////
        //add zhangenwei
        MEC_SERVICE_LOG_ERROR << "timestamp_millisecond-src:" << cictciEvent.m_Timestamp.c_str();
        header->set_timestamp_millisecond(TimeStampTransfer(cictciEvent.m_Timestamp));
        MEC_SERVICE_LOG_ERROR << "timestamp_millisecond-src:" << TimeStampTransfer(cictciEvent.m_Timestamp);
        MEC_SERVICE_LOG_ERROR << "timestamp_millisecond-pb :" << header->timestamp_millisecond();

        header->set_dev_no(cictciEvent.m_DevNo);
        if(cictciEvent.m_RadarNo_Empty)
        {
            header->set_radar_no(cictciEvent.m_RadarNo);
        }
        if(cictciEvent.m_LidarNo_Empty)
        {
            header->set_lidar_no(cictciEvent.m_LidarNo);
        }
        header->set_mec_no(cictciEvent.m_MecNo);
        MEC_SERVICE_LOG_ERROR << "[event-num]" << cictciEvent.m_Evt_List.size();
        /////////////////////////////////////////////////////////
        for (size_t i = 0; i < cictciEvent.m_Evt_List.size(); ++i)
        {
            airos::usecase::EventInformation *event = output_data_event_->add_events();
            auto cictciEventDetail = cictciEvent.m_Evt_List[i];
            event->set_id_str(cictciEventDetail.m_ID);
            event->set_id(SENSOROBJIDALLOCATOR_INSTANCE_REF.getObjId(cictciEventDetail.m_ID));
            event->set_event_type_mec(cictciEventDetail.m_EvtType);

            MEC_SERVICE_LOG_INFO << "Cictci event type: " << cictciEventDetail.m_EvtType;
            // add by lht for test platform 2.0
            auto it = kCictciToAirosMap_.find(static_cast<os::v2x::device::CICTCI_EventType>(cictciEventDetail.m_EvtType));
            if(it != kCictciToAirosMap_.end()) { 
                event->set_event_type(it->second);
            } else {
                MEC_SERVICE_LOG_ERROR << "[error] cictci event type not found, use NONE instead, type: " << cictciEventDetail.m_EvtType;
                event->set_event_type(airos::usecase::EventInformation_EventType_NONE);
            } 
            event->set_timestamp(TimeStampTransfer(cictciEvent.m_Timestamp) / 1000.0);  // s
            // end add by lht for test platform 2.0

            if (!cictciEventDetail.m_EvtStatus_Empty && 2 == cictciEventDetail.m_EvtStatus)
            {
                event->set_stop_flag(true);
            }
            else
            {
                event->set_stop_flag(false);
            }

            // 经纬度坐标转UTM坐标
            int zone;
            bool dummy;
            double x, y;
            try{
                GeographicLib::UTMUPS::Forward(cictciEventDetail.m_Lat, cictciEventDetail.m_Lon, zone, dummy, x, y);
            }catch(afl::util::Exception& e)
            {
                MEC_SERVICE_LOG_ERROR << "json par error " << e.what();
            }

            event->mutable_location_point()->set_x(x);
            event->mutable_location_point()->set_y(y);
            event->mutable_location_point()->set_z(cictciEventDetail.m_Ele);
            event->mutable_location_point()->set_zone(zone);
            ///////////////////
            //add zhangenwei
            event->mutable_position_gcs()->set_lon(cictciEventDetail.m_Lon);
            event->mutable_position_gcs()->set_lat(cictciEventDetail.m_Lat);
            if(!cictciEventDetail.m_Ele_Empty)
            {
                event->mutable_position_gcs()->set_ele(cictciEventDetail.m_Ele);
            }

            if(!cictciEventDetail.m_VehL_Empty)
            {
                event->set_vehicle_length(cictciEventDetail.m_VehL);
            }

            if(!cictciEventDetail.m_VehW_Empty)
            {
                event->set_vehicle_width(cictciEventDetail.m_VehW);
            }

            if(!cictciEventDetail.m_VehH_Empty)
            {
                event->set_vehicle_height(cictciEventDetail.m_VehH);
            }
            event->set_obj_id_str(cictciEventDetail.m_ObjID);

            if(!cictciEventDetail.m_EvtPtcType_Empty)
            {
                event->set_evt_ptc_type(cictciEventDetail.m_EvtPtcType);
            }

            if(!cictciEventDetail.m_VehType_Empty)
            {
                event->set_veh_type(cictciEventDetail.m_VehType);
            }

            if(!cictciEventDetail.m_NonVehType_Empty)
            {
                event->set_noveh_type(cictciEventDetail.m_NonVehType);
            }

            if(!cictciEventDetail.m_EvtV_Empty)
            {
                event->set_event_video(cictciEventDetail.m_EvtV);
            }

            if(!cictciEventDetail.m_EvtP_Empty)
            {
                event->set_event_picture(cictciEventDetail.m_EvtP);
            }
        }
    }catch(afl::util::Exception& e)
    {
        MEC_SERVICE_LOG_ERROR << "json par error " << e.what();
        MEC_IN_DEBUG_PRINT << "json par error " << e.what();
        return false;
    }

    MEC_SERVICE_LOG_WARN << "[mec-out]" << output_data_event_->ShortDebugString();
    sender_(output_data_event_);
    return true;
}
 
void CICTCIMec::Stop()
{
    stop_ = true;
    if (thread_process_recv_ != nullptr && thread_process_recv_->joinable())
    {
        thread_process_recv_->join();
    }
}


bool CICTCIMec::ParseJsonData2PbDataHeartBeat(json &j)
{
    CictciHeartBeat cictciHeartBeat;
    try
    {
        cictciHeartBeat  = j;
    }
    catch(json::exception &e)
    {
        MEC_SERVICE_LOG_ERROR << "(error) " << e.what();
        return false;
    }
    // MEC_SERVICE_LOG_INFO << j.dump().c_str();
    output_data_hearbeat_->Clear();
    auto heart_beat = output_data_hearbeat_->mutable_mec_heart_beat();
    heart_beat->set_msg_type(cictciHeartBeat.m_MsgType);
    heart_beat->set_time_stamp(TimeStampTransfer(cictciHeartBeat.m_Timestamp));
    for(int32_t i = 0; i < cictciHeartBeat.m_Dev_List.size(); i++)
    {
        auto cictciDevDetail = cictciHeartBeat.m_Dev_List[i];
        auto device_status_info =  heart_beat->add_device_status_info();

        uint8_t devType = 0;
        switch (cictciDevDetail.m_DevType)
        {
            case 1:
                devType =  airos::usecase::HEART_BEAT_TYPE_MEC;
                break;
            case 2:
                devType =  airos::usecase::HEART_BEAT_TYPE_CAMRA;
                break;
            case 3:
                devType =  airos::usecase::HEART_BEAT_TYPE_MW_RADAR;
                break;
            case 4:
                devType =  airos::usecase::HEART_BEAT_TYPE_RADAR;
                break;
            default:
                devType = airos::usecase::HEART_BEAT_TYPE_MEC;
                break;
        }

        device_status_info->set_device_type((airos::usecase::MecHeartBeatDeviceType)devType);
        MEC_SERVICE_LOG_ERROR << cictciDevDetail.m_DeviceId;
        device_status_info->set_device_id(cictciDevDetail.m_DeviceId);
        device_status_info->set_status((airos::usecase::MecHeartBeatDeviceStatus)cictciDevDetail.m_Status);
    }

    MEC_SERVICE_LOG_WARN << "[mec-out]" << output_data_hearbeat_->ShortDebugString();
    sender_(output_data_hearbeat_);
}
uint64_t CICTCIMec::TimeStampTransfer(std::string& timeInString)
{
    //input: string like "1970-01-01 00:00:00.000"
    //output: u64 millisecond number from 1970.01.01 00:00:00.000
    struct tm tm;

    if (23 != timeInString.length())
    {
        timeInString=timeInString+".000";
    }

    if (23 != timeInString.length())
    {
        return 0;
    }
    string yearInString (timeInString, 0, 4);
    string mouthInString (timeInString, 5, 2);
    string dayInString (timeInString, 8, 2);
    string hourInString (timeInString, 11, 2);
    string minuteInString (timeInString, 14, 2);
    string secondInString (timeInString, 17, 2);
    string millisecond (timeInString, 20, 3);

    int year = atoi(yearInString.c_str());
    if (1970 > year)
    {
        return 0;
    }
    int mouth = atoi(mouthInString.c_str());
    if ((1 > mouth) || (12 < mouth))
    {
        return 0;
    }
    int day = atoi(dayInString.c_str());
    if ((1 > day) || (31 < day))
    {
        return 0;
    }
    int hour = atoi(hourInString.c_str());
    if ((0 > hour) || (23 < hour))
    {
        return 0;
    }
    int minute = atoi(minuteInString.c_str());
    if ((0 > minute) || (59 < minute))
    {
        return 0;
    }
    int second = atoi(secondInString.c_str());
    if ((0 > second) || (59 < second))
    {
        return 0;
    }

    tm.tm_sec = second;
    tm.tm_min = minute;
    tm.tm_hour = hour;
    tm.tm_mday = day;
    tm.tm_mon = mouth - 1;
    tm.tm_year = year - 1900;
    tm.tm_isdst = -1;

    return (((uint64_t)mktime(&tm)) * 1000 + atoi(millisecond.c_str()));
}
bool CICTCIMec::ParseJsonData2PbDataAngleOffset(json &j)
{
    CictciAngleOffset cictciAngleOffset;
    try
    {
        cictciAngleOffset  = j;
    }
    catch(json::exception &e)
    {
        MEC_SERVICE_LOG_ERROR << "(error) " << e.what();
        MEC_IN_DEBUG_PRINT << "(error) " << e.what();
        return false;
    }
     MEC_SERVICE_LOG_INFO << j.dump().c_str();
    json test = cictciAngleOffset;
    MEC_SERVICE_LOG_INFO << test.dump().c_str();
    output_data_angle_offset_->Clear();
    auto mec_angle_offset = output_data_angle_offset_->mutable_mec_angle_offset();
    mec_angle_offset->set_msg_type(cictciAngleOffset.m_MsgType);
    mec_angle_offset->set_time_stamp(cictciAngleOffset.m_Timestamp);
    for(int i = 0; i < cictciAngleOffset.m_Alarm.size(); i++)
    {
        auto alarm = cictciAngleOffset.m_Alarm[i];
        auto alarmInfo = mec_angle_offset->add_alarm_info();
        alarmInfo->set_alarm_level(alarm.m_AlarmLevel);
        alarmInfo->set_alarm_status(alarm.m_AlarmStatus);
        alarmInfo->set_alarm_raised_time(alarm.m_AlarmRaisedTime);
        if(!alarm.m_AlarmChangedTime_Empty)
        {
            alarmInfo->set_alarm_changed_time(alarm.m_AlarmChangedTime);
        }

        alarmInfo->set_device_id(alarm.m_DeviceID);
        alarmInfo->set_horizontal_offset(alarm.m_HorizontalOffset);
        alarmInfo->set_vertical_offsetset(alarm.m_VerticalOffset);
    }
    MEC_SERVICE_LOG_WARN << " [mec-out] " << output_data_angle_offset_->ShortDebugString();
    sender_(output_data_angle_offset_);
}

bool CICTCIMec::ParseJsonData2PbDataTrafficlight(json &j)
{
    CictciTrafficlightList ctll;
    try
    {
        ctll  = j;
    }
    catch(json::exception &e)
    {
        MEC_SERVICE_LOG_ERROR << "(error) " << e.what();
        MEC_IN_DEBUG_PRINT << "(error) " << e.what();
        return false;
    }

    MEC_SERVICE_LOG_INFO << j.dump().c_str();
    json test = ctll;
    MEC_SERVICE_LOG_INFO << test.dump().c_str();

    output_data_trafficlight_list_->Clear();
    auto trafficlightlist = output_data_trafficlight_list_->mutable_mec_trafficlight_list();
    trafficlightlist->set_msg_type(ctll.m_MsgType);
    trafficlightlist->set_time_stamp(afl::util::TimeStamp::now(true).millSeconds());

    for(int i = 0; i < ctll.m_Trafficlight_List.size(); i++)
    {
        auto templightjson = ctll.m_Trafficlight_List[i];
        auto templightpb = trafficlightlist->add_trafficlight_list();
        templightpb->set_link_index(templightjson.m_LinkIndex);
        templightpb->set_trafficlight_type(templightjson.m_TrafficlightType);
        templightpb->set_color(templightjson.m_Color);
        templightpb->set_count(templightjson.m_Count);
    }
    MEC_SERVICE_LOG_WARN << output_data_trafficlight_list_->ShortDebugString();
    sender_(output_data_trafficlight_list_);
    return true;
}

bool CICTCIMec::ParseJsonData2PbDataTrafficFlow(json &j)
{
    CictciTrafficFlow ctf;
    try
    {
        ctf  = j;
    }
    catch(json::exception &e)
    {
        MEC_SERVICE_LOG_ERROR << "(error) " << e.what();
        MEC_IN_DEBUG_PRINT  << "(error) " << e.what();
        return false;
    }

    MEC_SERVICE_LOG_INFO << j.dump().c_str();
    json test = ctf;
    MEC_SERVICE_LOG_INFO << test.dump().c_str();
    output_data_trafficflow_->Clear();
    auto trafficflow = output_data_trafficflow_->mutable_mec_trafficflow();
    trafficflow->set_msg_type(ctf.m_MsgType);
    trafficflow->set_time_stamp(TimeStampTransfer(ctf.m_Timestamp));
    trafficflow->set_dev_no(ctf.m_DevNo);
    trafficflow->set_mec_no(ctf.m_MecNo);
    trafficflow->set_cycle(ctf.m_Cycle);
    trafficflow->set_coil_num(ctf.m_CoilNum);

    for(int i = 0; i < ctf.m_Coil_List.size(); i++)
    {
        auto tempflowjson = ctf.m_Coil_List[i];
        auto tempflowpb = trafficflow->add_trafficflow_list();
        tempflowpb->set_region_id(tempflowjson.m_RegionID);
        tempflowpb->set_node_id(tempflowjson.m_NodeID);
        tempflowpb->set_upstream_id(tempflowjson.m_UpstreamCrossID);
        tempflowpb->set_meas_no(tempflowjson.m_MeasNo);
        tempflowpb->set_lane_id(tempflowjson.m_LaneID);
        tempflowpb->set_lane_no(tempflowjson.m_LaneNo);
        tempflowpb->set_coil_no(tempflowjson.m_CoilNo);
        tempflowpb->set_volume(tempflowjson.m_Volume);
        tempflowpb->set_volume1(tempflowjson.m_Volume1);
        tempflowpb->set_volume2(tempflowjson.m_Volume2);
        tempflowpb->set_volume3(tempflowjson.m_Volume3);
        tempflowpb->set_volume4(tempflowjson.m_Volume4);
        tempflowpb->set_volume5(tempflowjson.m_Volume5);
        tempflowpb->set_pcu(tempflowjson.m_PCU);
        tempflowpb->set_av_speed(tempflowjson.m_AVSpeed);
        tempflowpb->set_occupancy(tempflowjson.m_Occupancy);
        tempflowpb->set_occupancy1(tempflowjson.m_Occupancy1);
        tempflowpb->set_headway(tempflowjson.m_Headway);
        tempflowpb->set_distance(tempflowjson.m_Distance);
        tempflowpb->set_gap(tempflowjson.m_Gap);
        tempflowpb->set_speed_85(tempflowjson.m_Speed_85);
        tempflowpb->set_delay(tempflowjson.m_Delay);
        tempflowpb->set_stop(tempflowjson.m_Stop);
        tempflowpb->set_desaturation(tempflowjson.m_Desaturation);
        tempflowpb->set_go_through(tempflowjson.m_Go_through);
        tempflowpb->set_queue_length(tempflowjson.m_Queue_length);
        tempflowpb->set_number(tempflowjson.m_Number);
        tempflowpb->set_pedestriancount(tempflowjson.m_PedestrianCount);
        tempflowpb->set_nonmotorcount(tempflowjson.m_NonMotorCount);
        tempflowpb->set_classqueuenaturalnumber(tempflowjson.m_ClassQueue_NaturalNumber);
        tempflowpb->set_classqueueequivalentnumber(tempflowjson.m_ClassQueue_EquivalentNumber);

    }

    MEC_SERVICE_LOG_WARN << "[mec-out]" <<output_data_trafficflow_->ShortDebugString();
    sender_(output_data_trafficflow_);
    return true;
}


V2XOS_MEC_REG_FACTORY(CICTCIMec, "cictci_mec");

}  // namespace device
}  // namespace v2x
}  // namespace os
