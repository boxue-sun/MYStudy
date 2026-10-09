#include "v2x_application.h"

#include <gflags/gflags.h>

#include "app/framework/interface/app_middleware.h"
#include "app/modules/v2x_application/common/app_flag.h"
#include "app/modules/v2x_application/usecase/map/map_info.h"
#include "base/common/log.h"

namespace airos {
namespace app {

bool V2xApplication::Init(const AppliactionConfig& conf) {


  std::string flag_file_path = conf.app_config_path() + "/v2x_config.flag";
  google::SetCommandLineOption("flagfile", flag_file_path.c_str());
  // cloud_adapter_ = std::make_shared<CloudAdapter>();
  // if (!cloud_adapter_->Init(&cloud_sequence_)) {
  //   APP_LOG_ERROR << "CLOUD ADAPTER init failed.";
  //   return false;
  // }

  // usecase_adapter_ =
  //     std::make_shared<airos::perception::usecase::UsecaseAdapter>();
  // if (!usecase_adapter_->Init(conf.app_config_path())) {
  //   APP_LOG_ERROR << "USECASE ADAPTER init failed.";
  //   return false;
  // }
  // RegisterOSMessage<airos::perception::PerceptionObstacles>(
  //     OSMessageType::PERCEPTION_OBSTACLES_SERVICE,
  //     std::bind(
  //         &airos::perception::usecase::UsecaseAdapter::Proc, usecase_adapter_,
  //         std::placeholders::_1));
  // perception_adapter_ = std::make_shared<PerceptionAdapter>();
  // if (!perception_adapter_->Init(&cloud_sequence_)) {
  //   APP_LOG_ERROR << "PERCEPTION ADAPTER init failed.";
  //   return false;
  // }
  // StartThreadPerceptionAdapter();
  std::string lnStr = "ln -s /home/airos/os/app/conf/airos_v2x_application/v2x_config.flag  /home/airos/common_config/v2x_config";
  system(lnStr.c_str());

  // 地图初始化
  map_generator_ = std::make_shared<MapGenerator>();
  if (FLAGS_enable_send_map) {
	  is_map_generator_init_ = map_generator_->Init(sender_, conf.app_config_path());
	  if (is_map_generator_init_) {
	      map_generator_->ThreadMap();
	  } else {
	      APP_LOG_ERROR << "Map generator init failed.";
	  }
  }

  // rsi初始化
  rsi_generator_ = std::make_shared<RsiGenerator>();
  if (FLAGS_enable_send_rsi) {
	is_rsi_generator_init_ = rsi_generator_->Init(sender_, conf.app_config_path());
  	if (!is_rsi_generator_init_) {
      APP_LOG_ERROR << "RSI GENERATOR init failed.";
  	}

  }
//   StartThreadRsiGenerator();

  // 信号机初始化
  traffic_light_adapter_ = std::make_shared<TrafficLightAdapter>();
  if (FLAGS_enable_send_spat) {
	  is_traffic_light_adapter_init_ =
	      traffic_light_adapter_->Init(&cloud_sequence_, sender_, conf.app_config_path());
	  if (is_traffic_light_adapter_init_) {
	      RegisterOSMessage<::airos::trafficlight::TrafficLightServiceData>(
	          OSMessageType::TRAFFIC_LIGHT_SERVICE,
	          std::bind(&V2xApplication::OnTrafficLightData, this, std::placeholders::_1));

	  } else {
	      APP_LOG_ERROR << "TRAFFIC LIGHT ADAPTER init failed.";
	  }
  }
  // mec初始化
  mec_adapter_ = std::make_shared<MECAdapter>();
  if (FLAGS_enable_send_rsm) {
    if (!mec_adapter_->Init(sender_)) {
        APP_LOG_ERROR << "MEC init failed.";
        return false;
      }
  }

  mec_adapter_ssm_ = std::make_shared<MECAdapterSsm>();
  if (FLAGS_enable_send_ssm) {
    if (!mec_adapter_ssm_->Init(sender_)) {
      APP_LOG_ERROR << "MEC adapter ssm init failed.";
      return false;
    }
  }

  RegisterOSMessage<airos::usecase::EventOutputResult>(
      OSMessageType::MEC_DATA,
      std::bind(
          &V2xApplication::OnMECData, this, std::placeholders::_1));

  // v2x_message_reporter_ = std::make_shared<V2xMessageReporter>();
  // if (!v2x_message_reporter_->Init(&cloud_sequence_)) {
  //   APP_LOG_ERROR << "V2X MESSAGE REPORTER init failed.";
  //   return false;
  // }
  // RegisterOSMessage<::os::v2x::device::RSUData>(
  //     OSMessageType::RSU_UPSTREAM,
  //     std::bind(
  //         &V2xMessageReporter::Proc, v2x_message_reporter_,
  //         std::placeholders::_1));
  // RegisterOSMessage<::os::v2x::device::RSUData>(
  //     OSMessageType::RSU_DOWNSTREAM,
  //     std::bind(
  //         &V2xMessageReporter::ProcRsuInData, v2x_message_reporter_,
  //         std::placeholders::_1));
  // monitor_ = std::make_shared<airos::monitor::Monitor>();
  // if (!monitor_->Init(&cloud_sequence_)) {
  //   APP_LOG_INFO << "MONITOR init failed.";
  //   return false;
  // }

  return true;
}

void V2xApplication::Start() {
    if (FLAGS_enable_send_rsi)
    {
        APP_LOG_INFO << "START.";
    }
}

void V2xApplication::OnTrafficLightData(
    const std::shared_ptr<const ::airos::trafficlight::TrafficLightServiceData>&
        data) {
  traffic_light_adapter_->Proc(data);
  airos::perception::usecase::SetTrafficLightState(data);
}

void V2xApplication::OnMECData(
    const std::shared_ptr<const airos::usecase::EventOutputResult>&
        data) {
    if (FLAGS_enable_send_ssm) {
        mec_adapter_ssm_->Proc(data);
    }

    if (FLAGS_enable_send_rsm) {
        mec_adapter_->Proc(data);
    }

    if (is_rsi_generator_init_) {
        rsi_generator_->Proc(data);
    }
    if (is_traffic_light_adapter_init_) {
        traffic_light_adapter_->Proc(data);
    }
}

void V2xApplication::StartThreadPerceptionAdapter() {
  b_perception_adapter_ = true;
  thread_perception_adapter_.reset(new std::thread([&] {
    while (b_perception_adapter_) {
      auto perception_data =
          std::make_shared<airos::usecase::EventOutputResult>();
      {
        std::unique_lock<std::mutex> guard(
            airos::perception::usecase::g_usecase_mtx);
        airos::perception::usecase::g_usecase_condition.wait(guard, [] {
          return airos::perception::usecase::g_usecase_for_perception
                     .ByteSizeLong() != 0;
        });
        perception_data->operator=(
            airos::perception::usecase::g_usecase_for_perception);
        airos::perception::usecase::g_usecase_for_perception.Clear();
      }
      perception_adapter_->Proc(perception_data);
    }
  }));
}

void V2xApplication::StartThreadRsiGenerator() {
  b_rsi_generator_ = true;
  thread_rsi_generator_.reset(new std::thread([&] {
    while (b_rsi_generator_) {
      auto rsi_data = std::make_shared<airos::usecase::EventOutputResult>();
      {
        std::unique_lock<std::mutex> guard(
            airos::perception::usecase::g_usecase_mtx);
        airos::perception::usecase::g_usecase_condition.wait(guard, [] {
          return airos::perception::usecase::g_uescase_for_rsi.ByteSizeLong() !=
                 0;
        });
        rsi_data->operator=(airos::perception::usecase::g_uescase_for_rsi);
        airos::perception::usecase::g_uescase_for_rsi.Clear();
      }
      rsi_generator_->Proc(rsi_data);
    }
  }));
}

V2xApplication::~V2xApplication() {
  if (thread_perception_adapter_ && thread_perception_adapter_->joinable()) {
    b_perception_adapter_ = false;
    thread_perception_adapter_->join();
    thread_perception_adapter_.reset();
      if (FLAGS_enable_send_rsi)
      {
          APP_LOG_INFO << __FUNCTION__ << "wait thread perception adapter.";
      }
  }

  if (thread_rsi_generator_ && thread_rsi_generator_->joinable()) {
    b_rsi_generator_ = false;
    thread_rsi_generator_->join();
    thread_rsi_generator_.reset();
      if (FLAGS_enable_send_rsi)
      {
          APP_LOG_INFO << __FUNCTION__ << "wait thread rsi generator.";
      }
  }
}

AIROS_APPLICATION_REG_FACTORY(V2xApplication, "appv2x")

}  // namespace app
}  // namespace airos
