#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "air_service/modules/perception-visualization/config/viz_config.pb.h"

namespace airos {
namespace perception {
namespace visualization {

class VizConfigManager {
 public:
  VizConfigManager();
  VizConfig& GetVizConfig() {
    return viz_config_;
  }

 private:
  bool LoadVizConfig(std::string viz_config_path);

  VizConfig viz_config_;
};

}  // namespace visualization
}  // namespace perception
}  // namespace airos
