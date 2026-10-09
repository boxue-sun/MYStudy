
#include "show_help.h"

#include <iostream>

#include <gflags/gflags.h>

namespace airos {
namespace pack {

void show_args(const std::string& type) {
  if (type.compare("create") == 0) {
    std::cout << "      -name    required,value is letters, numbers,the "
                 "underscore character and size <=50"
              << std::endl;
    std::cout << "      -type    required,value is [app camera_perception "
                 "lidar_perception fusion rsu traffic_light camera lidar radar]"
              << std::endl;
  }
  if (type.compare("make") == 0) {
    std::cout << "      -type    required,value is [cmake bazel]" << std::endl;
    std::cout << "      -name    optional,package name" << std::endl;
  }
  if (type.compare("release") == 0) {
    std::cout << "      -name    optional,package name" << std::endl;
    std::cout
        << "      -out    optional,default output.tar.gz format:xxx.tar.gz"
        << std::endl;
  }
  if (type.compare("install") == 0) {
    std::cout << "      -file    required,package file" << std::endl;
  }
  if (type.compare("remove") == 0) {
    std::cout << "      -name    required,package name" << std::endl;
  }
  if (type.compare("run") == 0) {
    std::cout
        << "      -name    required,value is the package name that is installed"
        << std::endl;
    std::cout << "      -type    required,value is [app camera_perception "
                 "lidar_perception fusion rsu traffic_light camera lidar radar]"
              << std::endl;
  }
  return;
}

void show_help() {
  std::cout << "Usage\n   airospkg [option] [-name] [-type] [-file] [-out]\n\n";
  std::cout << "option:\n";
  std::cout << "  create -name [package_name] -type [package_type]"
            << std::endl;
  show_args("create");
  std::cout << std::endl;
  std::cout << "  make -type [compile_type]  -name [package_name]" << std::endl;
  show_args("make");
  std::cout << std::endl;
  std::cout << "  release -name [package_name] -out [out_name]" << std::endl;
  show_args("release");
  std::cout << std::endl;
  std::cout << "  install -file [package_file]" << std::endl;
  show_args("install");
  std::cout << std::endl;
  std::cout << "  remove -name [package_name]" << std::endl;
  show_args("remove");
  std::cout << std::endl;
  std::cout << "  run [value]" << std::endl;
  show_args("run");
  return;
}

void show_help_create() {
  std::cout << "Usage\n   airospkg create -name [package_name] -type "
               "[package_type]\n\n";
  std::cout << "option:\n";
  show_args("create");
  return;
}

void show_help_make() {
  std::cout << "Usage\n   airospkg make -type [compile_type]  -name "
               "[package_name]\n\n";
  std::cout << "option:\n";
  show_args("make");
  return;
}

void show_help_release() {
  std::cout
      << "Usage\n   airospkg release -name [package_name] -out [out_name]\n\n";
  std::cout << "option:\n";
  show_args("release");
  return;
}

void show_help_install() {
  std::cout << "Usage\n   airospkg install -file [package_file]\n\n";
  std::cout << "option:\n";
  show_args("install");
  return;
}

void show_help_remove() {
  std::cout << "Usage\n   airospkg remove -name [package_name]\n\n";
  std::cout << "option:\n";
  show_args("remove");
  return;
}

void show_help_run() {
  std::cout
      << "Usage\n   airospkg run -type [package_type] -name [package_name]\n\n";
  std::cout << "option:\n";
  show_args("run");
  return;
}

bool have_help() {
  gflags::CommandLineFlagInfo info;
  if (GetCommandLineFlagInfo("h", &info) && !info.is_default) {
    return true;
  }

  if (GetCommandLineFlagInfo("help", &info) && !info.is_default) {
    return true;
  }

  return false;
}

}  // namespace pack
}  // namespace airos
