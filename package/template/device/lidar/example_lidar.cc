#include "example_lidar.h"

#include <yaml-cpp/yaml.h>

#include "base/common/log.h"
#include "base/device_connect/lidar/device_factory.h"

namespace os {
namespace v2x {
namespace device {

bool ExampleLidar::Init(const LidarInitConfig& config) {
  return true;
}

V2XOS_LIDAR_REG_FACTORY(ExampleLidar, "example_lidar");

}  // namespace device
}  // namespace v2x
}  // namespace os
