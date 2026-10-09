#include <iostream>
#include <set>
#include <string>

#include "airos_pack.h"
#include "show_help.h"

bool is_type_support(const std::string& pkg_name) {
  std::set<std::string> supported_types = {
      "app",
      "camera_perception",
      "camera_perception",
      "lidar_perception",
      "fusion",
      "rsu",
      "traffic_light",
      "camera",
      "lidar",
      "radar",
      };

  return supported_types.find(pkg_name) != supported_types.end();
}

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);
  if (argc < 2) {
    airos::pack::show_help();
    return 0;
  }

  std::string str_opt(argv[1]);
  if (str_opt.compare("create") == 0) {
    if (airos::pack::have_help()) {
      airos::pack::show_help_create();
      return 0;
    }
    std::string str_name(airos::pack::FLAGS_name);
    if (str_name.empty()) {
      std::cout << "[WARN]-name need value" << std::endl;
      airos::pack::show_help_create();
      return 0;
    }

    std::string str_type(airos::pack::FLAGS_type);
    if (str_type.empty()) {
      std::cout << "[WARN]-type need value" << std::endl;
      airos::pack::show_help_create();
      return 0;
    }

    airos::pack::en_tpl_type type = airos::pack::EN_TYPE_UNKNOW;
    if (str_type.compare("camera_perception") == 0) {
      type = airos::pack::EN_TYPE_CAMERA_PERCEPTION;
    }
    if (str_type.compare("lidar_perception") == 0) {
      type = airos::pack::EN_TYPE_LIDAR_PERCEPTION;
    }
    if (str_type.compare("fusion") == 0) {
      type = airos::pack::EN_TYPE_FUSION;
    }

    if (str_type.compare("app") == 0) {
      type = airos::pack::EN_TYPE_APP;
    }

    if (str_type.compare("rsu") == 0) {
      type = airos::pack::EN_TYPE_RSU;
    }
    if (str_type.compare("traffic_light") == 0) {
      type = airos::pack::EN_TYPE_TRAFFIC_LIGHT;
    }
    if (str_type.compare("camera") == 0) {
      type = airos::pack::EN_TYPE_CAMERA;
    }
    if (str_type.compare("lidar") == 0) {
      type = airos::pack::EN_TYPE_LIDAR;
    }
    if (str_type.compare("radar") == 0) {
      type = airos::pack::EN_TYPE_RADAR;
    }
    if (type == airos::pack::EN_TYPE_UNKNOW) {
      std::cout << "[WARN]-type invaild value: " << str_type << std::endl;
      airos::pack::show_help_create();
      return 0;
    }

    airos::pack::AirosPack apm;
    apm.create(str_name, type);
    return 0;
  }

  if (str_opt.compare("make") == 0) {
    if (airos::pack::have_help()) {
      airos::pack::show_help_make();
      return 0;
    }

    std::string str_type(airos::pack::FLAGS_type);
    if (str_type.empty()) {
      std::cout << "[WARN]-type need value" << std::endl;
      airos::pack::show_help_make();
      return 0;
    }
    airos::pack::AirosPack apm;
    apm.make();
    return 0;
  }

  if (str_opt.compare("release") == 0) {
    if (airos::pack::have_help()) {
      airos::pack::show_help_release();
      return 0;
    }

    airos::pack::AirosPack apm;
    apm.release();
    return 0;
  }

  if (str_opt.compare("install") == 0) {
    if (airos::pack::have_help()) {
      airos::pack::show_help_install();
      return 0;
    }
    if (airos::pack::FLAGS_file.empty()) {
      airos::pack::show_help_install();
      return 0;
    }

    airos::pack::AirosPack apm;
    apm.install(airos::pack::FLAGS_file);
    return 0;
  }

  if (str_opt.compare("remove") == 0) {
    if (airos::pack::have_help()) {
      airos::pack::show_help_remove();
      return 0;
    }

    airos::pack::AirosPack apm;
    std::string str_name(airos::pack::FLAGS_name);
    if (str_name.empty()) {
      std::cout << "[WARN]-name need value" << std::endl;
      airos::pack::show_help_remove();
      return 0;
    }
    apm.remove(str_name);
    return 0;
  }

  if (str_opt.compare("run") == 0) {
    if (airos::pack::have_help()) {
      airos::pack::show_help_run();
      return 0;
    }
    std::string str_name(airos::pack::FLAGS_name);
    if (str_name.empty()) {
      std::cout << "[WARN]-name need value" << std::endl;
      airos::pack::show_help_run();
      return 0;
    }

    std::string str_type(airos::pack::FLAGS_type);
    if (str_type.empty()) {
      std::cout << "[WARN]-type need value" << std::endl;
      airos::pack::show_help_run();
      return 0;
    }
    if (!is_type_support(str_type)) {
      std::cout << "[WARN]-type invaild value: " << str_type << std::endl;
      airos::pack::show_help_run();
      return 0;
    }
    airos::pack::AirosPack apm;
    apm.run(str_name, str_type);
    return 0;
  }

  return 0;
}
