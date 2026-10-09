#include "example_traffic_light.h"

#include <yaml-cpp/yaml.h>

#include "base/common/log.h"
#include "base/device_connect/traffic_light/device_factory.h"

namespace os {
namespace v2x {
namespace device {
bool ExampleTrafficLight::Init(const std::string& config_file) {
  std::string ip;
  try {
    YAML::Node root_node = YAML::LoadFile(config_file);
    ip                   = root_node["ip"].as<std::string>();
    /*
      init device config
    */
  } catch (const std::exception& err) {
    LOG_ERROR << err.what();
    return false;
  } catch (...) {
    LOG_ERROR << "Parse yaml config failed.";
    return false;
  }
  LOG_INFO << "traffic light device ip: " << ip;
  return true;
}

void ExampleTrafficLight::Start() {
  auto data = std::make_shared<TrafficLightBaseData>();
  /*
    fill data
  */
  sender_(data);
  return;
}
void ExampleTrafficLight::WriteToDevice(
    const std::shared_ptr<const TrafficLightReceiveData>& re_proto) {
  return;
}
V2XOS_TRAFFIC_LIGHT_REG_FACTORY(ExampleTrafficLight, "example_traffic_light");
}  // namespace device
}  // namespace v2x
}  // namespace os
