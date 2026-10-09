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

#pragma once

#include <memory>
#include <string>
#include "cyber/cyber.h"
#include "cyber/component/component.h"
#include "cyber/node/node.h"
#include "cyber/record/record_reader.h" // 如果需要从 record 文件读取数据
#include "cyber/record/record_writer.h" // 如果需要将数据写入 record 文件
#include "air_service/framework/proto/airos_traffic_light.pb.h"
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "base/device_connect/proto/cloud_data.pb.h"

#include "app/framework/interface/app_base.h"
#include "app/modules/v2x_application/common/producer_consumer_queue.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "middleware/protocol/proto/monitor_mec.pb.h"
#include "base/common/network/date_time.h"
using namespace afl::util;
namespace airos {
namespace app {

struct MecPhaseInfo
{
    int color;
    int count;
};

struct LastPhaseInfo
{
    uint64_t updatetime;
    int color;
    int count;
    int countunchangetime;//倒计时卡秒时长
};

class TrafficLightAdapter
{
public:
    TrafficLightAdapter()
        : spat_count_(1)
    {
        output_data_status_trafficlight_ = std::make_shared<airos::monitor_mec::MonitorSpat>();
    }

    ~TrafficLightAdapter(){};
    bool Init(
        ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
            cloud_sequence,
        const airos::app::ApplicationCallBack& send_cb,
        const std::string& conf_path);
    bool Proc(
        const std::shared_ptr<const airos::trafficlight::TrafficLightServiceData>&
            service_data);
    bool Proc(
        const std::shared_ptr<const airos::usecase::EventOutputResult>&
                  trafficlightlist_ptr);

private:
    bool TrafficLightServicePb2Cloud(
        const std::shared_ptr<const airos::trafficlight::TrafficLightServiceData>&
            service_data,
        std::shared_ptr<os::v2x::device::CloudData> cloud_pb);
    bool TrafficLightServicePb2AsnPb(
        const std::shared_ptr<const airos::trafficlight::TrafficLightServiceData>&
            service_data,
        std::shared_ptr<v2xpb::asn::MessageFrame> asn_pb);
    int64_t get_minute_year();
    int64_t get_mill_second_minute();
    bool send_step_control(int step);

    bool isColorCorrect(int phaseid, int color);
    bool isCountCorrect(int phaseid, int color, int count, int& faultcode);

    int spat_count_;
    // max PhaseID 255
    static const uint32_t MAX_VEHICLE_PHASE = 200;
    std::string rscu_sn_;
    std::string MQTT_TRAFFICLIGHT_TOPIC_PREFIX = "upload/trafficlight/";
    ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
        cloud_sequence_;
    airos::app::ApplicationCallBack send_;
    std::map<int, int> phaseId_convert_map_;//<mec_phase_id, standard_phase_id>
    std::map<int, LastPhaseInfo> last_trafficlights_map_;
    std::map<int, MecPhaseInfo> mec_trafficlights_map;
    uint64_t mec_recv_time;
    std::shared_ptr<apollo::cyber::Writer<airos::usecase::EventOutputResult>> writer_;
    std::string channel_name = "/v2x/mec/om/spat/detect/data_";
    std::shared_ptr <airos::usecase::EventOutputResult> output_data_trafficlight_detect_ = nullptr;
    uint64_t normal_time = 0;
    //信号机数据状态
    std::shared_ptr<apollo::cyber::Writer<airos::monitor_mec::MonitorSpat>> writer_trafficlght_data_status_;
    std::string channel_name_trafficlight_data_status = "/v2x/mec/om/spat/status_";
    std::shared_ptr <airos::monitor_mec::MonitorSpat> output_data_status_trafficlight_ = nullptr;
};

}  // namespace app
}  // namespace airos
