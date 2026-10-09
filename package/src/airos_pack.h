#pragma once

#include <iostream>
#include <map>
#include <string>

#include "create_template.h"
#include "maco_params.h"

namespace airos {
namespace pack {

class AirosPack {
 private:
  std::string dir_create_{MACO_DIR_CREATE};

  std::string dir_install_{MACO_DIR_INSTALL};
  std::string dir_install_app_{MACO_DIR_INSTALL_APP};
  std::string dir_install_perception_{MACO_DIR_INSTALL_PERCEPTION};
  std::string dir_install_fusion_{MACO_DIR_INSTALL_FUSION};
  std::string dir_install_rsu_{MACO_DIR_INSTALL_RSU};
  std::string dir_install_traffic_light_{MACO_DIR_INSTALL_TRAFFIC_LIGHT};
  std::string dir_install_camera_{MACO_DIR_INSTALL_CAMERA};
  std::string dir_install_lidar_{MACO_DIR_INSTALL_LIDAR};
  std::string dir_install_radar_{MACO_DIR_INSTALL_RADAR};

  bool check_pack_name(const std::string& pname);
  const std::vector<std::string> parse_verion_file(const std::string& fname);
  std::map<std::string, std::string> cfg_read(const std::string& fname);
  void cfg_write(
      const std::string& fname, const std::map<std::string, std::string> data);
  std::string stringtrim(const std::string& str);
  bool remove(const std::string& dir_install, const std::string& pname);
  bool make_bazel();
  bool make_cmake();
  bool shell_exec(const std::string& cmd);
  bool modify_run_cfg(const std::string& pkg_name, const std::string& pkg_type);

 public:
  AirosPack();
  ~AirosPack();

  bool create(const std::string& pname, en_tpl_type type);
  bool make();
  bool release();
  bool install(const std::string& pfile);
  bool remove(const std::string& pname);
  bool run(const std::string& pname, const std::string& type);
};

}  // namespace pack
}  // namespace airos
