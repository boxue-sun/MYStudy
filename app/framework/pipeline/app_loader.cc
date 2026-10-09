#include "app/framework/pipeline/app_loader.h"

#include <map>
#include <iostream>

#include <boost/any.hpp>

#include "air_service/framework/proto/airos_perception_obstacle.pb.h"
#include "air_service/framework/proto/airos_traffic_light.pb.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "base/device_connect/proto/rsu_data.pb.h"

#include "app/framework/interface/app_base.h"
#include "base/common/log.h"
#include "base/plugin/modules_loader/dynamic_loader.h"

namespace airos {
namespace app {

using TrafficLightServiceData = ::airos::trafficlight::TrafficLightServiceData;
using PerceptionServiceData   = airos::perception::PerceptionObstacles;
using RSUData                 = os::v2x::device::RSUData;
using MesssageFrame           = v2xpb::asn::MessageFrame;
using MECData                 = airos::usecase::EventOutputResult;

void AppLoader::InitChannelMapping() {
  channel_mapping_ = {
      {OSMessageType::TRAFFIC_LIGHT_SERVICE,
       "/airos/service/traffic_light/data"},
      {OSMessageType::PERCEPTION_OBSTACLES_SERVICE,
       "/airos/perception/obstacles"},
      {OSMessageType::RSU_UPSTREAM, "/airos/device/rsu_out"},
      {OSMessageType::RSU_DOWNSTREAM, "/airos/device/rsu_in"},
      {OSMessageType::VISUAL_TRAFFIC_LIGHT, "/airos/visual_trafficlight"},
      {OSMessageType::MEC_DATA, "/v2x/usecase"},
      {OSMessageType::V2X_MESSAGE_RECEIVED, "/airos/message/received"},
      {OSMessageType::V2X_MESSAGE_SEND, "/airos/message/generated"}};

  const int camera_start =
      static_cast<int>(OSMessageType::PERCEPTION_OBSTACLES_CAMERA1);
  const int camera_end =
      static_cast<int>(OSMessageType::PERCEPTION_OBSTACLES_CAMERA20);

  for (int i = camera_start; i <= camera_end; ++i) {
    OSMessageType type = static_cast<OSMessageType>(i);
    std::string channel =
        "/airos/perception/obstacles_camera" + std::to_string(i - 10);
    channel_mapping_[type] = channel;
  }

  const int lidar_start =
      static_cast<int>(OSMessageType::PERCEPTION_OBSTACLES_LIDAR1);
  const int lidar_end =
      static_cast<int>(OSMessageType::PERCEPTION_OBSTACLES_LIDAR4);

  for (int i = lidar_start; i <= lidar_end; ++i) {
    OSMessageType type = static_cast<OSMessageType>(i);
    std::string channel =
        "/airos/lidar/obstacles_lidar" + std::to_string(i - 50);
    channel_mapping_[type] = channel;
  }
}

bool AppLoader::LoadAppliaction(
    const std::string& app_path, const AppliactionConfig& conf,
    const ApplicationCallBack& cb) {
  airos::base::DynamicLoader::GetInstance().LoadAppPackage(app_path);
  // get class
  app_ = ApplicationFactory::Instance().GetUnique(conf.app_name(), cb);
  if (app_ == nullptr) {
    APP_LOG_ERROR << "Get application failed: " << conf.app_name();
    return false;
  }
  std::cout << "Get App: " << conf.app_name() << std::endl;

  app_->Init(conf);

  std::string node_name = "app_" + conf.app_name();
  node_ = std::make_shared<airos::middleware::AirMiddlewareNode>(node_name);

  auto reader_list = GetSubscribedMessageTypes();
  for (auto& reader : reader_list) {
    CreateAirosReader(reader, node_);
  }
  app_->Start();
  return true;
}

void AppLoader::CreateAirosReader(
    OSMessageType type,
    std::shared_ptr<airos::middleware::AirMiddlewareNode>& node) {
  switch (type) {
    case OSMessageType::TRAFFIC_LIGHT_SERVICE:
      CreateReaderHelper<TrafficLightServiceData>(type, node);
      break;
    case OSMessageType::PERCEPTION_OBSTACLES_SERVICE:
      CreateReaderHelper<PerceptionServiceData>(type, node);
      break;
    case OSMessageType::RSU_UPSTREAM:
    case OSMessageType::RSU_DOWNSTREAM:
      CreateReaderHelper<RSUData>(type, node);
      break;
    case OSMessageType::MEC_DATA:
      CreateReaderHelper<MECData>(type, node);
      break;
    case OSMessageType::V2X_MESSAGE_RECEIVED:
      CreateReaderHelper<MesssageFrame>(type, node);
      break;
    case OSMessageType::V2X_MESSAGE_SEND:
      CreateReaderHelper<MesssageFrame>(type, node);
      break;
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA1:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA2:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA3:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA4:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA5:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA6:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA7:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA8:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA9:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA10:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA11:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA12:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA13:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA14:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA15:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA16:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA17:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA18:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA19:
    case OSMessageType::PERCEPTION_OBSTACLES_CAMERA20:
      CreateReaderHelper<PerceptionServiceData>(type, node);
      break;
    case OSMessageType::PERCEPTION_OBSTACLES_LIDAR1:
    case OSMessageType::PERCEPTION_OBSTACLES_LIDAR2:
    case OSMessageType::PERCEPTION_OBSTACLES_LIDAR3:
    case OSMessageType::PERCEPTION_OBSTACLES_LIDAR4:
      CreateReaderHelper<PerceptionServiceData>(type, node);
      break;
    default:
      break;
  }
  return;
}

}  // namespace app
}  // namespace airos
