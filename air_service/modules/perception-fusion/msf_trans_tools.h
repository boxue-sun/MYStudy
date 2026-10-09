#pragma once

#include <cmath>
#include <thread>

#include "air_service/framework/proto/airos_perception_obstacle.pb.h"
#include "air_service/modules/perception-fusion/proto/multi_sensor_fusion_config.pb.h"

#include "air_service/modules/perception-fusion/algorithm/interface/multi_sensor_fusion.h"

namespace airos {
namespace perception {
namespace msf {
namespace tools {

class TransTools {
 public:
  TransTools()  = default;
  ~TransTools() = default;

  static void PbToObject(
      const PerceptionObstacle& obstacle, const std::string& frame_id,
      msf::PerceptionObject* object);

  static bool PbsToObjects(
      const airos::perception::PerceptionObstacles& obstacles,
      msf::FusionInput* msg);

  static void ObjectsToPbs(
      const msf::FusionOutput& msg,
      airos::perception::PerceptionObstacles* obstacles);

  static void ObjectToPb(
      const msf::FusionObject& object,
      airos::perception::PerceptionObstacle* obstacle);

  static void FillObjectPolygonFromBBox3D(
      airos::perception::PerceptionObstacle* object_ptr);

 private:
  static void GetDefaultVar(
      const PerceptionObstacle& obstacle, Eigen::Matrix3d* var);
};

}  // namespace tools
}  // namespace msf
}  // namespace perception
}  // namespace airos
