#pragma once

#include <string>

#include "app/framework/proto/airos_app_config.pb.h"

#include "app/framework/interface/app_factory.h"
#include "app/framework/interface/app_middleware.h"
#include "middleware/runtime/src/air_middleware_node.h"

namespace airos {
namespace app {

class AppLoader {
 public:
  AppLoader() {
    InitChannelMapping();
  };
  bool LoadAppliaction(
      const std::string& app_path, const AppliactionConfig& conf,
      const ApplicationCallBack& cb);

 private:
  void InitChannelMapping();
  template <typename MessageT>
  void CreateReaderHelper(
      OSMessageType type,
      std::shared_ptr<airos::middleware::AirMiddlewareNode>& node) {
    node->CreateReader<MessageT>(
        channel_mapping_.at(type),
        std::bind(&TriggerMessage<MessageT>, type, std::placeholders::_1));
  }

  void CreateAirosReader(
      OSMessageType type,
      std::shared_ptr<airos::middleware::AirMiddlewareNode>& node);

  std::map<OSMessageType, std::string> channel_mapping_;
  std::shared_ptr<AirOSApplication> app_;
  std::shared_ptr<airos::middleware::AirMiddlewareNode> node_;
};

}  // namespace app
}  // namespace airos
