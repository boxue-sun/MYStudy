#pragma once

#include "gflags/gflags.h"

// #define MACO_DIR_TEMPLATE "/airos/package/template/"
// #define MACO_DIR_CREATE "/airos/package/"

#define MACO_DIR_TEMPLATE "/opt/airos/package/template/"
#define MACO_DIR_CREATE "/home/airos/projs/"

#define MACO_DIR_INSTALL "/home/airos/os/"
#define MACO_DIR_INSTALL_APP "/home/airos/os/app/lib/"
#define MACO_DIR_INSTALL_PERCEPTION "/home/airos/os/modules/lib/"
#define MACO_DIR_INSTALL_FUSION "/home/airos/os/modules/lib/"
#define MACO_DIR_INSTALL_RSU "/home/airos/os/device/lib/rsu/"
#define MACO_DIR_INSTALL_TRAFFIC_LIGHT \
  "/home/airos/os/device/lib/traffic_light/"
#define MACO_DIR_INSTALL_CAMERA "/home/airos/os/device/lib/camera/"
#define MACO_DIR_INSTALL_LIDAR "/home/airos/os/device/lib/lidar/"
#define MACO_DIR_INSTALL_RADAR "/home/airos/os/device/lib/radar/"

namespace airos {
namespace pack {

DECLARE_string(name);
DECLARE_string(type);
DECLARE_string(file);
DECLARE_bool(ques);
DECLARE_string(out);
DECLARE_bool(h);

enum en_tpl_type {
  EN_TYPE_UNKNOW = 0,
  EN_TYPE_CAMERA_PERCEPTION,
  EN_TYPE_LIDAR_PERCEPTION,
  EN_TYPE_FUSION,
  EN_TYPE_APP,
  EN_TYPE_RSU,
  EN_TYPE_TRAFFIC_LIGHT,
  EN_TYPE_CAMERA,
  EN_TYPE_LIDAR,
  EN_TYPE_RADAR,
};

}  // namespace pack
}  // namespace airos
