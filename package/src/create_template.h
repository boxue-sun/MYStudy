#pragma once

#include <iostream>
#include <string>

#include "maco_params.h"

namespace airos {
namespace pack {

class CreateTemplate {
 private:
  std::string dir_tpe_{MACO_DIR_TEMPLATE};

 public:
  CreateTemplate() {}
  ~CreateTemplate() {}

  bool create(
      const std::string& dir, const std::string& name, en_tpl_type type);
  bool template_camera_perception(
      const std::string& dir, const std::string& name);
  bool template_lidar_perception(
      const std::string& dir, const std::string& name);
  bool template_fusion(const std::string& dir, const std::string& name);
  bool template_app(const std::string& dir, const std::string& name);
  bool template_rsu(const std::string& dir, const std::string& name);
  bool template_traffic_light(const std::string& dir, const std::string& name);
  bool template_camera(const std::string& dir, const std::string& name);
  bool template_lidar(const std::string& dir, const std::string& name);
  bool template_radar(const std::string& dir, const std::string& name);

  void file_replace(const std::string& fname, const std::string& name);
};
}  // namespace pack
}  // namespace airos
