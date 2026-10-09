#include "app/framework/pipeline/send_controller.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "base/common/log.h"

namespace airos {
namespace app {

SendController::SendController() {
  boost::uuids::uuid r_uuid = boost::uuids::random_generator()();
  std::string send_node = "SendController_" + boost::uuids::to_string(r_uuid);
  node_.reset(new airos::middleware::AirMiddlewareNode(send_node));
  asn_writer_ = node_->CreateWriter<v2xpb::asn::MessageFrame>(
      std::string("/airos/message/generated"));
}

void SendController::SendToCloud(const airos::cloud::CloudFrame& cloud_frame) {
  // pass
}

void SendController::AppSendCallback(
    const ::airos::app::ApplicationDataType& app_data) {
  if (!app_data) {
    APP_LOG_WARN << "ptr null";
    return;
  }
  // TODO: send rate ctrl
  switch (app_data->payload_case()) {
    case airos::app::ApplicationData::PayloadCase::kRoadSideFrame:
      asn_writer_->Write(std::make_shared<v2xpb::asn::MessageFrame>(
          app_data->road_side_frame()));
      break;
    case airos::app::ApplicationData::PayloadCase::kCloudMessageFrame:
      SendToCloud(app_data->cloud_message_frame());
      break;
    default:
      break;
  }
}

}  // namespace app
}  // namespace airos
