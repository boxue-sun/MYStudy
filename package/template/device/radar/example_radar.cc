#include "example_radar.h"

#include <yaml-cpp/yaml.h>

#include "base/common/log.h"
#include "base/device_connect/radar/device_factory.h"

namespace os {
namespace v2x {
namespace device {

bool ExampleRadar::Init(const std::string& config_file) {
  LOG_INFO << "example radar init";
  return true;
}

void ExampleRadar::Start() {
  LOG_INFO << "example radar start";
}

V2XOS_RADAR_REG_FACTORY(ExampleRadar, "example_radar");

}  // namespace device
}  // namespace v2x
}  // namespace os
