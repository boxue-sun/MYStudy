/******************************************************************************
 * Copyright 2022 The Airos Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "traffic_light_adapter.h"

#include <sys/time.h>

#include <vector>

#include "base/env/env.h"
#include "base/common/network/time_stamp.h"
#include "base/common/log.h"
#include "yaml-cpp/yaml.h"

namespace airos {
namespace app {
bool TrafficLightAdapter::Init(
    ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
    cloud_sequence,
    const airos::app::ApplicationCallBack& send_cb,
    const std::string&                     conf_path)
{
    rscu_sn_        = airos::base::Environment::GetDeviceSn();
    cloud_sequence_ = cloud_sequence;
    send_           = send_cb;


    // 创建节点
    auto node = apollo::cyber::CreateNode("traffic_light_adapter");
    if (!node)
    {
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "[error]Failed to create node.";
        return false;
    }
    else
    {
        // 创建 Writer
        writer_ = node->CreateWriter<airos::usecase::EventOutputResult>(channel_name);

        if (!writer_)
        {
            TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "[error]Failed to create writer for channel: " << channel_name;
            // 使用 AINFO 替代 std::cout
            return false;
        }
    }
    // 创建节点
    auto node_spat = apollo::cyber::CreateNode("traffic_light_adapter_data");
    if (!node_spat)
    {
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "[error]Failed to create node.";
        return false;
    }
    else
    {
        // 创建 Writer
        writer_trafficlght_data_status_ = node_spat->CreateWriter<airos::monitor_mec::MonitorSpat>(channel_name_trafficlight_data_status);

        if (!writer_trafficlght_data_status_)
        {
            TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "[error]Failed to create writer for channel: " << channel_name;
            // 使用 AINFO 替代 std::cout
            return false;
        }
    }
    output_data_trafficlight_detect_        = std::make_shared<airos::usecase::EventOutputResult>();
    std::string TrafficLightAdapter_configs = conf_path + "/traffic_light_adapter.yaml";
    YAML::Node  root_node                   = YAML::LoadFile(TrafficLightAdapter_configs);
    if (!root_node.IsMap())
    {
        std::cout << "traffic_light_adapter Init ConfigManager Failed!" << std::endl;
        return false;
    }

    if (root_node["phase_id_list"])
    {
        const YAML::Node& list = root_node["phase_id_list"];
        for (const auto& phase_pair : list)
        {
            int radarphaseId                   = phase_pair["mec_phase_id"].as<int>();
            int standardphaseId                = phase_pair["standard_phase_id"].as<int>();
            phaseId_convert_map_[radarphaseId] = standardphaseId;
        }
    }
    else
    {
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "get peer address list failed";
        return false;
    }

    return true;
}

bool TrafficLightAdapter::Proc(
    const std::shared_ptr<const airos::trafficlight::TrafficLightServiceData>&
    service_data)
{
    auto asn_pb = std::make_shared<v2xpb::asn::MessageFrame>();
    if (TrafficLightServicePb2AsnPb(service_data, asn_pb))
    {
        auto message_pb = std::make_shared<airos::app::ApplicationData>();
        message_pb->mutable_road_side_frame()->operator=(*asn_pb);
        send_(message_pb);

        if (writer_trafficlght_data_status_)
        {
            output_data_status_trafficlight_->Clear();
            auto md_spat_data_status = output_data_status_trafficlight_->mutable_md_spat_data_status();
            md_spat_data_status->set_tag(airos::monitor_mec::MonitorSpatTag::MONITOR_TAG_MONITOR_OUT_SPAT_DATA_STATUS);
            md_spat_data_status->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
            md_spat_data_status->set_status(airos::monitor_mec::MonitorSpatDataStatus::MONITOR_STATUS_SPAT_DATA_NORMAL);
            writer_trafficlght_data_status_->Write(output_data_status_trafficlight_);
        }
    }
    else
    {
        auto md_spat_data_status = output_data_status_trafficlight_->mutable_md_spat_data_status();
        md_spat_data_status->set_tag(airos::monitor_mec::MonitorSpatTag::MONITOR_TAG_MONITOR_OUT_SPAT_DATA_STATUS);
        md_spat_data_status->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
        md_spat_data_status->set_status(airos::monitor_mec::MonitorSpatDataStatus::MONITOR_STATUS_SPAT_DATA_UNNORMAL);
        writer_trafficlght_data_status_->Write(output_data_status_trafficlight_);
    }
    if (send_step_control(2))
    {
        auto cloud_pb = std::make_shared<os::v2x::device::CloudData>();
        if (TrafficLightServicePb2Cloud(service_data, cloud_pb))
        {
            cloud_sequence_->push(cloud_pb);
        }
    }
    return true;
}

bool TrafficLightAdapter::TrafficLightServicePb2Cloud(
    const std::shared_ptr<const airos::trafficlight::TrafficLightServiceData>&
    service_data,
    std::shared_ptr<os::v2x::device::CloudData> cloud_pb)
{
    if (!service_data || !cloud_pb)
    {
        return false;
    }
    auto mqtt_pb = cloud_pb->mutable_mqtt_data();
    mqtt_pb->set_topic(MQTT_TRAFFICLIGHT_TOPIC_PREFIX + rscu_sn_);
    std::string str_data;
    service_data->SerializePartialToString(&str_data);

    mqtt_pb->set_data(str_data);
    return true;
}

bool TrafficLightAdapter::TrafficLightServicePb2AsnPb(
    const std::shared_ptr<const airos::trafficlight::TrafficLightServiceData>&
    service_data,
    std::shared_ptr<v2xpb::asn::MessageFrame> asn_pb)
{
    if (!service_data || !asn_pb)
    {
        return false;
    }

    const int pb_phase_cnt = service_data->traffic_light().phase().size();
    if (pb_phase_cnt == 0)
    {
        return false;
    }

    auto spat   = asn_pb->mutable_spatframe();
    spat_count_ = (spat_count_ >= 127) ? 1 : (spat_count_ + 1);
    spat->set_message_count(spat_count_);
    spat->set_dsecond(get_mill_second_minute());

    auto inter = spat->add_intersections();
    inter->set_node_region(service_data->region_id());
    inter->set_node_id(service_data->cross_id());
    inter->set_moy(get_minute_year());
    inter->set_dsecond(get_mill_second_minute());

    int  phase_index        = 0;
    int  temptimeconfidence = 10;
    int  tl_status          = 0;
    bool has_green          = false;
    bool has_red            = false;
    bool has_yellow         = false;
    while (phase_index < pb_phase_cnt)
    {
        auto& light_phase = service_data->traffic_light().phase()[phase_index++];
        if (static_cast<uint32_t>(light_phase.light_id()) > MAX_VEHICLE_PHASE)
        {
            continue;
        }
        auto phase = inter->add_phases();
        phase->set_id(light_phase.light_id());

        airos::trafficlight::LightState light_state        = light_phase.light_status();
        bool                            is_light_unchanged = light_phase.light_unchanged();
        int32_t                         light_type         = light_phase.light_type();

        std::vector<int32_t> period(3, 0);
        int32_t              light_count_down = 0;
        if (is_light_unchanged != true)
        {
            for (int i = 0; (i < light_phase.step_info_list().size()) && (i < 3);
                 i++)
            {
                period[i] = light_phase.step_info_list(i).duration();
            }
            // 正常有倒计时，异常情况：短暂无倒计时、非常亮情况下无倒计时、初始常亮学习阶段
            if (light_phase.has_count_down())
            {
                light_count_down = light_phase.count_down();
            }
        }
        else
        {
            // 常亮灯时倒计时赋值99
            period[0]        = 99;
            light_count_down = 99;
        }

        switch (light_state)
        {
        case airos::trafficlight::LightState::RED:
        case airos::trafficlight::LightState::FLASHING_RED:
            has_red = true;
            break;
        case airos::trafficlight::LightState::GREEN:
        case airos::trafficlight::LightState::FLASHING_GREEN:
            has_green = true;
            break;
        case airos::trafficlight::LightState::YELLOW:
        case airos::trafficlight::LightState::FLASHING_YELLOW:
            has_yellow = true;
            break;
        default:
            break;
        }

        // 黄灯常亮
        if (is_light_unchanged &&
            (light_state == airos::trafficlight::LightState::YELLOW))
        {
            light_state = airos::trafficlight::LightState::FLASHING_YELLOW;
        }

        bool is_protected_green = false;
        if ((light_type == airos::trafficlight::LightType::STRAIGHT_DIRECTION) ||
            (light_type == airos::trafficlight::LightType::LEFT_DIRECTION) ||
            (light_type == airos::trafficlight::LightType::RIGHT_DIRECTION) ||
            (light_type == airos::trafficlight::LightType::TURN_DIRECTION))
        {
            is_protected_green = true;
        }

        bool is_manual = false;
        if (light_count_down == 0 ||
            service_data->traffic_light().device_info().control_mode() ==
            airos::trafficlight::ControlMode::LOCAL_MANUAL)
        {
            is_manual = true;
        }


        // 单相位无倒计时、倒计时为0、信号机手控时，SPAT不填倒计时，且每个相位只有一态
        if (is_manual)
        {
            phase->add_phase_state();
        }
        else
        {
            for (int i = 0; i < 3; i++)
            {
                phase->add_phase_state();
            }
        }

        //黑灯严重性>灯色异常>倒计时异常
        //黑灯
        if (light_state == airos::trafficlight::LightState::DARK)
        {
            tl_status = 1;
        }

        if (tl_status == 0 && temptimeconfidence != 0)
        {
            int  currentphaseid = light_phase.light_id();
            bool coloravailable = true;
            bool countavailable = true;
            int  fault_code     = 0; //当前相位异常值，0正常 1黑灯 2灯色冲突 3倒计时与现场不一致 4卡秒 5跳秒 6回跳 7切灯倒计时不为1
            coloravailable      = isColorCorrect(currentphaseid, light_state);
            countavailable      = isCountCorrect(currentphaseid, light_state, light_count_down, fault_code);

            if (!coloravailable) //当前相位灯色异常
            {
                temptimeconfidence = 0;
                fault_code         = 2;
            }
            else
            {
                if (!countavailable) //当前相位灯色无异常，倒计时异常
                {
                    temptimeconfidence = 5;
                }
            }

            if (fault_code != 0) //当前相位异常时才可能赋值
            {
                if (tl_status == 0) //已判断相位均无异常时，赋值
                {
                    tl_status = fault_code;
                }
                else //已判断相位有异常时，可能赋值
                {
                    if (tl_status > fault_code) //当前相位异常，且异常严重性大于(异常值小于)已判断相位时，赋值
                    {
                        tl_status = fault_code;
                    }
                }
            }
            APP_LOG_INFO << "tlstatus: " << tl_status << " faultcode: " << fault_code;
        }

        switch (light_state)
        {
        case airos::trafficlight::LightState::GREEN: // 绿灯
        case airos::trafficlight::LightState::FLASHING_GREEN:
            phase->mutable_phase_state(0)->set_color(
                is_protected_green
                    ? v2xpb::asn::COLOR_PROTECTED_GREEN
                    : v2xpb::asn::COLOR_PERMISSIVE_GREEN);
            if (!is_manual)
            {
                phase->mutable_phase_state(0)->set_timing_start(0);
                phase->mutable_phase_state(0)->set_timing_end(light_count_down);
                phase->mutable_phase_state(0)->set_timing_duration(period[0]);

                phase->mutable_phase_state(1)->set_color(v2xpb::asn::COLOR_YELLOW);
                phase->mutable_phase_state(1)->set_timing_start(
                    phase->mutable_phase_state(0)->timing_end());
                phase->mutable_phase_state(1)->set_timing_end(
                    phase->mutable_phase_state(1)->timing_start() + period[1]);
                phase->mutable_phase_state(1)->set_timing_duration(period[1]);

                phase->mutable_phase_state(2)->set_color(v2xpb::asn::COLOR_RED);
                phase->mutable_phase_state(2)->set_timing_start(
                    phase->mutable_phase_state(1)->timing_end());
                phase->mutable_phase_state(2)->set_timing_end(
                    phase->mutable_phase_state(2)->timing_start() + period[2]);
                phase->mutable_phase_state(2)->set_timing_duration(period[2]);
            }
            break;
        case airos::trafficlight::LightState::RED: // 红灯
        case airos::trafficlight::LightState::FLASHING_RED:
            phase->mutable_phase_state(0)->set_color(v2xpb::asn::COLOR_RED);
            if (!is_manual)
            {
                phase->mutable_phase_state(0)->set_timing_start(0);
                phase->mutable_phase_state(0)->set_timing_end(light_count_down);
                phase->mutable_phase_state(0)->set_timing_duration(period[0]);

                phase->mutable_phase_state(1)->set_color(
                    is_protected_green
                        ? v2xpb::asn::COLOR_PROTECTED_GREEN
                        : v2xpb::asn::COLOR_PERMISSIVE_GREEN);
                phase->mutable_phase_state(1)->set_timing_start(
                    phase->mutable_phase_state(0)->timing_end());
                phase->mutable_phase_state(1)->set_timing_end(
                    phase->mutable_phase_state(1)->timing_start() + period[1]);
                phase->mutable_phase_state(1)->set_timing_duration(period[1]);

                phase->mutable_phase_state(2)->set_color(v2xpb::asn::COLOR_YELLOW);
                phase->mutable_phase_state(2)->set_timing_start(
                    phase->mutable_phase_state(1)->timing_end());
                phase->mutable_phase_state(2)->set_timing_end(
                    phase->mutable_phase_state(2)->timing_start() + period[2]);
                phase->mutable_phase_state(2)->set_timing_duration(period[2]);
            }
            break;
        case airos::trafficlight::LightState::YELLOW: // 黄灯
            phase->mutable_phase_state(0)->set_color(v2xpb::asn::COLOR_YELLOW);
            if (!is_manual)
            {
                phase->mutable_phase_state(0)->set_timing_start(0);
                phase->mutable_phase_state(0)->set_timing_end(light_count_down);
                phase->mutable_phase_state(0)->set_timing_duration(period[0]);

                phase->mutable_phase_state(1)->set_color(v2xpb::asn::COLOR_RED);
                phase->mutable_phase_state(1)->set_timing_start(
                    phase->mutable_phase_state(0)->timing_end());
                phase->mutable_phase_state(1)->set_timing_end(
                    phase->mutable_phase_state(1)->timing_start() + period[1]);
                phase->mutable_phase_state(1)->set_timing_duration(period[1]);

                phase->mutable_phase_state(2)->set_color(
                    is_protected_green
                        ? v2xpb::asn::COLOR_PROTECTED_GREEN
                        : v2xpb::asn::COLOR_PERMISSIVE_GREEN);
                phase->mutable_phase_state(2)->set_timing_start(
                    phase->mutable_phase_state(1)->timing_end());
                phase->mutable_phase_state(2)->set_timing_end(
                    phase->mutable_phase_state(2)->timing_start() + period[2]);
                phase->mutable_phase_state(2)->set_timing_duration(period[2]);
            }
            break;
        case airos::trafficlight::LightState::FLASHING_YELLOW: // 黄闪
            phase->mutable_phase_state(0)->set_color(
                v2xpb::asn::COLOR_FLASHING_YELLOW);
            if (!is_manual)
            {
                phase->mutable_phase_state(0)->set_timing_start(0);
                phase->mutable_phase_state(0)->set_timing_end(light_count_down);
                phase->mutable_phase_state(0)->set_timing_duration(period[0]);

                phase->mutable_phase_state(1)->set_color(v2xpb::asn::COLOR_RED);
                phase->mutable_phase_state(1)->set_timing_start(
                    phase->mutable_phase_state(0)->timing_end());
                phase->mutable_phase_state(1)->set_timing_end(
                    phase->mutable_phase_state(1)->timing_start() + period[1]);
                phase->mutable_phase_state(1)->set_timing_duration(period[1]);

                phase->mutable_phase_state(2)->set_color(
                    is_protected_green
                        ? v2xpb::asn::COLOR_PROTECTED_GREEN
                        : v2xpb::asn::COLOR_PERMISSIVE_GREEN);
                phase->mutable_phase_state(2)->set_timing_start(
                    phase->mutable_phase_state(1)->timing_end());
                phase->mutable_phase_state(2)->set_timing_end(
                    phase->mutable_phase_state(2)->timing_start() + period[2]);
                phase->mutable_phase_state(2)->set_timing_duration(period[2]);
            }
            break;
        default:
          break;
            // return false;
        }
    }
    if(tl_status == 0)//无其他故障时判断全红/绿/黄
    {
      if (has_red && !has_green && !has_yellow) //全红
      {
          tl_status = 8;
      }
      else if (has_green && !has_red && !has_yellow) //全绿
      {
          tl_status = 9;
      }
      else if (has_yellow && !has_green && !has_red) //黄闪
      {
          tl_status = 10;
      }
      else
      {

      }
    }

    if (writer_)
    {
        output_data_trafficlight_detect_->Clear();
        auto header = output_data_trafficlight_detect_->mutable_header();

        ///////////////////////////////////////////////
        auto* trafficlight_detect_data = output_data_trafficlight_detect_->mutable_trafficlight_detect_data();
        trafficlight_detect_data->set_time_stamp(afl::util::TimeStamp::now(true).millSeconds());
        trafficlight_detect_data->set_trafficlight_status((airos::usecase::TrafficLightStatus)tl_status);
        writer_->Write(output_data_trafficlight_detect_);
    }
    else
    {
        APP_LOG_INFO << "Writer is not initialized!";
    }

    inter->set_time_confidence(temptimeconfidence);

    return true;
}

bool TrafficLightAdapter::send_step_control(int step)
{
    static int occurrences = 0;
    if (++occurrences > step)
    {
        occurrences -= step;
    }
    if (occurrences == 1)
    {
        return true;
    }
    return false;
}

int64_t TrafficLightAdapter::get_minute_year()
{
    struct tm* t         = nullptr;
    time_t     startTime = time(0);

    struct tm buf = {};
    localtime_r(&startTime, &buf);
    t = &buf;

    if (t == nullptr)
    {
        return -1;
    }
    return (t->tm_yday * 60 * 24 + t->tm_hour * 60 + t->tm_min);
}

int64_t TrafficLightAdapter::get_mill_second_minute()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0)
    {
        return -1;
    }
    struct tm* t         = nullptr;
    time_t     startTime = time(0);

    struct tm buf = {};
    localtime_r(&startTime, &buf);
    t = &buf;

    if (t == nullptr)
    {
        return -1;
    }
    return (t->tm_sec * 1000 + tv.tv_usec / 1000);
}

bool TrafficLightAdapter::Proc(
    const std::shared_ptr<const airos::usecase::EventOutputResult>&
    trafficlightlist_ptr)
{
    if (!trafficlightlist_ptr)
    {
        APP_LOG_INFO << "trafficlightlist_ptr is empty";
        return false;
    }

    if (!trafficlightlist_ptr->has_mec_trafficlight_list())
    {
        APP_LOG_INFO << "trafficlightlist_ptr is exist, but list is empty";
        return false;
    }

    auto lights = trafficlightlist_ptr->mec_trafficlight_list();

    mec_recv_time = lights.time_stamp();
    for (int i = 0; i < lights.trafficlight_list().size(); i++)
    {
        auto templight = lights.trafficlight_list()[i];
        int  tempid, tempcolor, tmepcount;
        if (phaseId_convert_map_.find(lights.trafficlight_list()[i].link_index()) == phaseId_convert_map_.end())
        {
            continue;
        }
        tempid    = phaseId_convert_map_[lights.trafficlight_list()[i].link_index()];
        tempcolor = lights.trafficlight_list()[i].color();
        tmepcount = lights.trafficlight_list()[i].count();
        MecPhaseInfo mpi;
        mpi.color = tempcolor;
        mpi.count = tmepcount;

        mec_trafficlights_map[tempid] = mpi;
    }
}

bool TrafficLightAdapter::isColorCorrect(int phaseid, int color)
{
    uint64_t tnow = afl::util::TimeStamp::now(true).millSeconds();
    if (tnow - mec_recv_time > 1500) //mec红绿灯数据超过1.5s未更新则不做对比
    {
        return true;
    }

    if (mec_trafficlights_map.find(phaseid) == mec_trafficlights_map.end())
    {
        APP_LOG_INFO << "phaseid not found by mec";
        return true;
    }

    if (color == airos::trafficlight::LightState::UNAVAILABLE)
    {
        return false;
    }

    bool colorcorrent = true;
    int  meccolor     = mec_trafficlights_map[phaseid].color;
    switch (color)
    {
    case airos::trafficlight::LightState::DARK:
        if (meccolor != 1)
        {
            colorcorrent = false;
        }
        break;
    case airos::trafficlight::LightState::RED:
    case airos::trafficlight::LightState::FLASHING_RED:
        if (meccolor != 2 && meccolor != 3)
        {
            colorcorrent = false;
        }
        break;
    case airos::trafficlight::LightState::GREEN:
    case airos::trafficlight::LightState::FLASHING_GREEN:
        if (meccolor != 4 && meccolor != 5)
        {
            colorcorrent = false;
        }
        break;
    case airos::trafficlight::LightState::YELLOW:
    case airos::trafficlight::LightState::FLASHING_YELLOW:
        if (meccolor != 7 && meccolor != 8)
        {
            colorcorrent = false;
        }
        break;
    default:
        break;
    }

    return colorcorrent;
}

bool TrafficLightAdapter::isCountCorrect(int phaseid, int color, int count, int& faultcode)
{
    switch (color)
    {
    case airos::trafficlight::LightState::FLASHING_RED:
        color = airos::trafficlight::LightState::RED;
        break;
    case airos::trafficlight::LightState::FLASHING_GREEN:
        color = airos::trafficlight::LightState::GREEN;
        break;
    case airos::trafficlight::LightState::FLASHING_YELLOW:
        color = airos::trafficlight::LightState::YELLOW;
        break;
    default:
        break;
    }

    uint64_t      tnow = afl::util::TimeStamp::now(true).millSeconds();
    LastPhaseInfo lpi;
    lpi.updatetime = tnow;
    lpi.color      = color;
    lpi.count      = count;

    //信号机倒计时自检
    if (last_trafficlights_map_.find(phaseid) == last_trafficlights_map_.end())
    {
        lpi.countunchangetime = 0;
        APP_LOG_INFO << "[detect]lpi.countunchangetime = 0";
    }
    else
    {
        //卡秒
        if (last_trafficlights_map_[phaseid].color == color && last_trafficlights_map_[phaseid].count == count)
        {
            lpi.countunchangetime = last_trafficlights_map_[phaseid].countunchangetime + (tnow - last_trafficlights_map_
                [phaseid].updatetime);
            if (lpi.countunchangetime > 1500)
            {
                APP_LOG_ERROR << "[detect][error]卡秒 > 1500ms";
                last_trafficlights_map_[phaseid] = lpi;
                faultcode                        = 4;
                return false;
            }
        }
        else
        {
            lpi.countunchangetime = 0;
        }

        //跳秒
        if (last_trafficlights_map_[phaseid].color == color && (last_trafficlights_map_[phaseid].count - count > 1))
        {
            APP_LOG_ERROR << "[detect][error]跳秒";
            last_trafficlights_map_[phaseid] = lpi;
            faultcode                        = 5;
            return false;
        }

        //回跳
        if (last_trafficlights_map_[phaseid].color == color && (last_trafficlights_map_[phaseid].count < count))
        {
            APP_LOG_ERROR << "[detect][error]回跳";
            last_trafficlights_map_[phaseid] = lpi;
            faultcode                        = 6;
            return false;
        }


        //切灯时倒计时不为1
        if (last_trafficlights_map_[phaseid].color != color && (last_trafficlights_map_[phaseid].count != 1))
        {
            APP_LOG_ERROR << "[detect][error]切灯时倒计时不为1";
            last_trafficlights_map_[phaseid] = lpi;
            faultcode                        = 7;
            return false;
        }
    }

    //感知与信号机倒计时一致性
    if (tnow - mec_recv_time < 1500)
    {
        APP_LOG_ERROR << "[detect][error]感知与信号机倒计时一致性";
        if (mec_trafficlights_map.find(phaseid) != mec_trafficlights_map.end())
        {
            if (abs(mec_trafficlights_map[phaseid].count - count) > 1) //感知与信号机倒计时差超过1s（不包含1s）认为倒计时不可用
            {
                last_trafficlights_map_[phaseid] = lpi;
                faultcode                        = 3;
                return false;
            }
        }
    }

    last_trafficlights_map_[phaseid] = lpi;
    return true;
}
} // namespace app
} // namespace airos
