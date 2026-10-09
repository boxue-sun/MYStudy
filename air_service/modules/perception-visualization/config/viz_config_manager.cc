#include "air_service/modules/perception-visualization/config/viz_config_manager.h"

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "air_service/modules/perception-visualization/config/viz_flags.h"
#include "base/common/log.h"
#include "base/io/protobuf_util.h"
#include "gflags/gflags.h"

namespace airos {
namespace perception {
namespace visualization {

VizConfigManager::VizConfigManager() {
  LoadVizConfig(FLAGS_viz_config_path);
}

bool VizConfigManager::LoadVizConfig(std::string viz_config_path) {
  return airos::base::ParseProtobufFromFile(viz_config_path, &viz_config_);
}

}  // namespace visualization
}  // namespace perception
}  // namespace airos
