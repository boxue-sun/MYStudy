
#include "create_template.h"

#include <fstream>
#include <sstream>

namespace airos {
namespace pack {

bool CreateTemplate::create(
    const std::string& dir, const std::string& name, en_tpl_type type) {
  std::stringstream ss;
  ss << "mkdir -p " << dir << "/conf";
  ss << "|mkdir -p " << dir << "/lib";
  if (system(ss.str().data()) < 0) {
    std::cout << "cmd false: " << __LINE__;
    return false;
  }

  if (type == EN_TYPE_CAMERA_PERCEPTION) {
    template_camera_perception(dir, name);
  }

  if (type == EN_TYPE_LIDAR_PERCEPTION) {
    template_lidar_perception(dir, name);
  }

  if (type == EN_TYPE_FUSION) {
    template_fusion(dir, name);
  }

  if (type == EN_TYPE_APP) {
    template_app(dir, name);
  }

  if (type == EN_TYPE_RSU) {
    template_rsu(dir, name);
  }
  if (type == EN_TYPE_TRAFFIC_LIGHT) {
    template_traffic_light(dir, name);
  }
  if (type == EN_TYPE_CAMERA) {
    template_camera(dir, name);
  }
  if (type == EN_TYPE_LIDAR) {
    template_lidar(dir, name);
  }
  if (type == EN_TYPE_RADAR) {
    template_radar(dir, name);
  }
  return true;
}

bool CreateTemplate::template_camera_perception(
    const std::string& dir, const std::string& name) {
  std::stringstream ss;
  ss << "cp -rf " << dir_tpe_ << "/perception/perception-camera/* " << dir;
  ss << "|cp -rf " << dir_tpe_ << "/perception/perception-camera/.bazel* "
     << dir;
  if (system(ss.str().data()) < 0) {
    std::cout << "cmd false: " << __LINE__;
    return false;
  }

  return true;
}
bool CreateTemplate::template_lidar_perception(
    const std::string& dir, const std::string& name) {
  std::stringstream ss;
  ss << "cp -rf " << dir_tpe_ << "/perception/perception-lidar/* " << dir;
  ss << "|cp -rf " << dir_tpe_ << "/perception/perception-lidar/.bazel* "
     << dir;
  if (system(ss.str().data()) < 0) {
    std::cout << "cmd false: " << __LINE__;
    return false;
  }

  return true;
}
bool CreateTemplate::template_fusion(
    const std::string& dir, const std::string& name) {
  std::stringstream ss;
  ss << "cp -rf " << dir_tpe_ << "/perception/perception-fusion/* " << dir;
  ss << "|cp -rf " << dir_tpe_ << "/perception/perception-fusion/.bazel* "
     << dir;
  if (system(ss.str().data()) < 0) {
    std::cout << "cmd false: " << __LINE__;
    return false;
  }

  return true;
}

bool CreateTemplate::template_app(
    const std::string& dir, const std::string& name) {
  std::stringstream ss;
  ss << "cp -rf " << dir_tpe_ << "/app/* " << dir;
  ss << "|cp -rf " << dir_tpe_ << "/app/.bazel* " << dir;
  if (system(ss.str().data()) < 0) {
    std::cout << "cmd false: " << __LINE__;
    return false;
  }

  return true;
}

bool CreateTemplate::template_rsu(
    const std::string& dir, const std::string& name) {
  std::stringstream ss;
  ss << "cp -rf " << dir_tpe_ << "/device/rsu/* " << dir;
  ss << "|cp -rf " << dir_tpe_ << "/device/rsu/.bazel* " << dir;
  if (system(ss.str().data()) < 0) {
    std::cout << "cmd false: " << __LINE__;
    return false;
  }

  return true;
}

bool CreateTemplate::template_traffic_light(
    const std::string& dir, const std::string& name) {
  std::stringstream ss;
  ss << "cp -rf " << dir_tpe_ << "/device/traffic_light/* " << dir;
  ss << "|cp -rf " << dir_tpe_ << "/device/traffic_light/.bazel* " << dir;
  if (system(ss.str().data()) < 0) {
    std::cout << "cmd false: " << __LINE__;
    return false;
  }

  return true;
}

bool CreateTemplate::template_camera(
    const std::string& dir, const std::string& name) {
  std::stringstream ss;
  ss << "cp -rf " << dir_tpe_ << "/device/camera/* " << dir;
  ss << "|cp -rf " << dir_tpe_ << "/device/camera/.bazel* " << dir;
  if (system(ss.str().data()) < 0) {
    std::cout << "cmd false: " << __LINE__;
    return false;
  }

  return true;
}

bool CreateTemplate::template_lidar(
    const std::string& dir, const std::string& name) {
  std::stringstream ss;
  ss << "cp -rf " << dir_tpe_ << "/device/lidar/* " << dir;
  ss << "|cp -rf " << dir_tpe_ << "/device/lidar/.bazel* " << dir;
  if (system(ss.str().data()) < 0) {
    std::cout << "cmd false: " << __LINE__;
    return false;
  }

  return true;
}

bool CreateTemplate::template_radar(
    const std::string& dir, const std::string& name) {
  std::stringstream ss;
  ss << "cp -rf " << dir_tpe_ << "/device/radar/* " << dir;
  ss << "|cp -rf " << dir_tpe_ << "/device/radar/.bazel* " << dir;
  if (system(ss.str().data()) < 0) {
    std::cout << "cmd false: " << __LINE__;
    return false;
  }

  return true;
}

void CreateTemplate::file_replace(
    const std::string& fname, const std::string& name) {
  // read
  std::fstream fst;
  fst.open(fname, std::fstream::in);
  if (!fst.good()) {
    return;
  }
  std::stringstream ss;
  ss << fst.rdbuf();
  fst.close();
  std::string data(ss.str());

  // replace
  std::string::size_type pos = 0;
  std::string sub_str("xxx");
  while ((pos = data.find(sub_str)) != std::string::npos) {
    data.replace(pos, sub_str.length(), name);
  }

  // write
  fst.open(fname, std::fstream::out | std::fstream::trunc);
  if (!fst.good()) {
    return;
  }
  fst << data;
  fst.close();

  return;
}

}  // namespace pack
}  // namespace airos
