#include "example_rsu.h"

#include <yaml-cpp/yaml.h>

#include "base/common/log.h"
#include "base/device_connect/rsu/device_factory.h"

namespace os {
namespace v2x {
namespace device {

bool ExampleRSU::Init(const std::string& config_file) {
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
  LOG_INFO << "rsu ip: " << ip;
  return true;
}

void ExampleRSU::Start() {
  auto data = std::make_shared<RSUData>();
  /*
    fill data
  */
  sender_(data);
  return;
}

void ExampleRSU::WriteToDevice(const std::shared_ptr<const RSUData>& re_proto) {
  return;
}

V2XOS_RSU_REG_FACTORY(ExampleRSU, "example_rsu");

}  // namespace device
}  // namespace v2x
}  // namespace os
