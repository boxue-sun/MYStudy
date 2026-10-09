#pragma once

#include "app/framework/proto/airos_app_data.pb.h"

#include "app/framework/interface/app_base.h"
#include "middleware/runtime/src/air_middleware_node.h"

namespace airos {
namespace app {

class SendController {
 public:
  SendController();
  void AppSendCallback(const ::airos::app::ApplicationDataType& app_data);
  void SendToCloud(const airos::cloud::CloudFrame& cloud_frame);

 private:
  std::unique_ptr<airos::middleware::AirMiddlewareNode> node_;
  std::shared_ptr<
      airos::middleware::AirMiddlewareWriter<v2xpb::asn::MessageFrame>>
      asn_writer_;
};

}  // namespace app
}  // namespace airos
