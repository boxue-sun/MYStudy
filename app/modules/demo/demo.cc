#include "air_service/framework/proto/airos_traffic_light.pb.h"

#include "app/framework/interface/app_base.h"
#include "app/framework/interface/app_factory.h"
#include "app/framework/interface/app_middleware.h"

namespace airos {
namespace app {

class Demo : public airos::app::AirOSApplication {
 public:
  Demo(const ApplicationCallBack& cb)
      : AirOSApplication(cb) {}

  ~Demo() {}

  bool Init(const AppliactionConfig& conf) {
    std::cout << "app path:" << conf.app_package_path() << std::endl;
    RegisterOSMessage<::airos::trafficlight::TrafficLightServiceData>(
        OSMessageType::TRAFFIC_LIGHT_SERVICE,
        std::bind(&Demo::OnTrafficLightData, this, std::placeholders::_1));
    return true;
  };

  void Start() override {
    auto data          = std::make_shared<ApplicationData>();
    auto message_frame = data->mutable_road_side_frame();
    auto spat_frame    = message_frame->mutable_spatframe();
    spat_frame->set_message_count(1);
    sender_(data);
    std::cout << "Hello World!" << std::endl;
  }

  void OnTrafficLightData(
      const std::shared_ptr<
          const ::airos::trafficlight::TrafficLightServiceData>& message) {
    std::cout << "Traffic Light Data" << std::endl;
  }
};

AIROS_APPLICATION_REG_FACTORY(Demo, "appdemo")
}  // namespace app
}  // namespace airos
