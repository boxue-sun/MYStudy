
#include "visualization_trans_tool.h"

#include <iostream>
#include <limits>
#include <unordered_map>

#include "Eigen/Core"
#include "Eigen/Eigenvalues"
#include "Eigen/Geometry"
#include "air_service/modules/perception-fusion/algorithm/air_fusion/base/common.h"
#include "air_service/modules/perception-fusion/algorithm/interface/multi_sensor_fusion.h"
#include "base/common/log.h"
#include "base/common/singleton.h"
// #include "libs/common/utils/config_para.h"

namespace airos {
namespace perception {
namespace visualization {
namespace tools {

using airos::perception::PerceptionObstacle;

const std::unordered_map<
    airos::perception::SubType, perception::PerceptionObstacle::Type>
    SubType2Type{
        {airos::perception::SubType::UNKNOWN,
         perception::PerceptionObstacle::UNKNOWN},
        {airos::perception::SubType::UNKNOWN_MOVABLE,
         perception::PerceptionObstacle::UNKNOWN},
        {airos::perception::SubType::UNKNOWN_UNMOVABLE,
         perception::PerceptionObstacle::UNKNOWN},
        {airos::perception::SubType::CAR,
         perception::PerceptionObstacle::VEHICLE},
        {airos::perception::SubType::VAN,
         perception::PerceptionObstacle::VEHICLE},
        {airos::perception::SubType::TRUCK,
         perception::PerceptionObstacle::VEHICLE},
        {airos::perception::SubType::BUS,
         perception::PerceptionObstacle::VEHICLE},
        {airos::perception::SubType::CYCLIST,
         perception::PerceptionObstacle::BICYCLE},
        {airos::perception::SubType::MOTORCYCLIST,
         perception::PerceptionObstacle::BICYCLE},
        {airos::perception::SubType::TRICYCLIST,
         perception::PerceptionObstacle::BICYCLE},
        {airos::perception::SubType::PEDESTRIAN,
         perception::PerceptionObstacle::PEDESTRIAN},
        {airos::perception::SubType::TRAFFICCONE,
         perception::PerceptionObstacle::UNKNOWN_UNMOVABLE},
        {airos::perception::SubType::SAFETY_TRIANGLE,
         perception::PerceptionObstacle::UNKNOWN_UNMOVABLE},
        {airos::perception::SubType::MAX_OBJECT_TYPE,
         perception::PerceptionObstacle::UNKNOWN_UNMOVABLE},
        {airos::perception::SubType::BARRIER_DELINEATOR,
         perception::PerceptionObstacle::UNKNOWN_UNMOVABLE},
        {airos::perception::SubType::BARRIER_WATER,
         perception::PerceptionObstacle::UNKNOWN_UNMOVABLE}};

static const msf::base::ObjectSubType typeID2Type[] = {
    msf::base::ObjectSubType::CAR,
    msf::base::ObjectSubType::TRUCK,
    msf::base::ObjectSubType::VAN,
    msf::base::ObjectSubType::BUS,
    msf::base::ObjectSubType::PEDESTRIAN,
    msf::base::ObjectSubType::CYCLIST,
    msf::base::ObjectSubType::TRICYCLIST,
    msf::base::ObjectSubType::MOTORCYCLIST,
    msf::base::ObjectSubType::UNKNOWN,
    msf::base::ObjectSubType::TRAFFICCONE,
    msf::base::ObjectSubType::UNKNOWN_UNMOVABLE,
    msf::base::ObjectSubType::UNKNOWN_MOVABLE,
    msf::base::ObjectSubType::SAFETY_TRIANGLE,
    msf::base::ObjectSubType::BARRIER_DELINEATOR,
    msf::base::ObjectSubType::BARRIER_WATER};

inline msf::base::ObjectSubType DetectObjectType(int type_id) {
  if (type_id < 0 || type_id >= 15) {
    return msf::base::ObjectSubType::MAX_OBJECT_TYPE;
  }
  return typeID2Type[type_id];
}

void TransTools::GetDefaultVar(
    const PerceptionObstacle& obstacle, Eigen::Matrix3d* var) {
  var->setZero();
  for (int i = 0; i < 2; ++i) {
    (*var)(i, i) = 1.0;
  }

  switch (obstacle.sub_type()) {
    case airos::perception::SubType::UNKNOWN:
      *var = *var * 3;
      break;

    case airos::perception::SubType::UNKNOWN_MOVABLE:
      *var = *var * 2;
      break;

    case airos::perception::SubType::UNKNOWN_UNMOVABLE:
      *var = *var * 2;
      break;

    case airos::perception::SubType::CAR:
      *var = *var * 1.6;
      break;

    case airos::perception::SubType::VAN:
      *var = *var * 1.6;
      break;

    case airos::perception::SubType::TRUCK:
      *var = *var * 3.5;
      break;

    case airos::perception::SubType::BUS:
      *var = *var * 3.5;
      break;

    case airos::perception::SubType::CYCLIST:
      *var = *var * 0.8;
      break;

    case airos::perception::SubType::MOTORCYCLIST:
      *var = *var * 0.8;
      break;

    case airos::perception::SubType::TRICYCLIST:
      *var = *var * 0.8;
      break;

    case airos::perception::SubType::PEDESTRIAN:
      *var = *var * 0.8;
      break;

    case airos::perception::SubType::TRAFFICCONE:
      *var = *var * 0.8;
      break;

    case airos::perception::SubType::SAFETY_TRIANGLE:
      *var = *var * 0.8;
      break;

    default:
      break;
  }
  if (obstacle.has_occ_state() &&
      obstacle.occ_state() ==
          airos::perception::OcclusionState::OCC_PARTIAL_OCCLUDED) {
    *var = *var * 1.5;
  } else if (
      obstacle.has_occ_state() &&
      obstacle.occ_state() ==
          airos::perception::OcclusionState::OCC_COMPLETE_OCCLUDED) {
    *var = *var * 2.5;
  }
}

void TransTools::PbToObject(
    const PerceptionObstacle& obstacle, const std::string& frame_id,
    msf::PerceptionObject* object) {
  object->sensor_type = msf::base::SensorType::MONOCULAR_CAMERA;
  object->timestamp   = obstacle.timestamp();
  object->sensor_name = frame_id;
  object->track_id    = obstacle.id();
  object->sub_type = static_cast<msf::base::ObjectSubType>(obstacle.sub_type());
  object->type =
      static_cast<msf::base::ObjectType>(SubType2Type.at(obstacle.sub_type()));

  std::vector<float> default_type_probs(6, 0);  // TODO
  std::vector<float> default_sub_type_probs(16, 0);
  for (int i = 0; i < obstacle.sub_type_probs_size(); ++i) {
    float sub_prob = obstacle.sub_type_probs(i).prob();
    // i is typeid
    auto sub_type =
        static_cast<airos::perception::SubType>(DetectObjectType(i));
    auto type = SubType2Type.at(sub_type);
    default_type_probs[static_cast<int>(type)] += sub_prob;
  }
  object->type_probs = default_type_probs;
  object->sub_type_probs.push_back(obstacle.sub_type_id_confidence());

  object->box.left_top.x     = obstacle.bbox2d().xmin();
  object->box.left_top.y     = obstacle.bbox2d().ymin();
  object->box.right_bottom.x = obstacle.bbox2d().xmax();
  object->box.right_bottom.y = obstacle.bbox2d().ymax();

  object->occ_state =
      static_cast<msf::base::OcclusionState>(obstacle.occ_state());
  object->center(0) = obstacle.position().x();
  object->center(1) = obstacle.position().y();
  object->center(2) = obstacle.position().z();

  GetDefaultVar(obstacle, &object->center_uncertainty);

  object->theta          = obstacle.theta();
  object->theta_variance = obstacle.theta_variance();

  object->size.length = obstacle.length();
  object->size.width  = obstacle.width();
  object->size.height = obstacle.height();

  if (obstacle.trunc_state() == TruncationState::TRUNC_TRUE) {
    object->is_truncation = true;
  } else {
    object->is_truncation = false;
  }
}

void TransTools::PbToObjectOutput(
    const airos::perception::PerceptionObstacle& obstacle,
    msf::FusionObject* object) {
  object->timestamp     = obstacle.timestamp();
  object->track_id      = obstacle.id();
  object->tracking_time = obstacle.tracking_time();
  object->center        = {
      obstacle.position().x(), obstacle.position().y(),
      obstacle.position().z()};
  object->theta       = obstacle.theta();
  object->size.length = obstacle.length();
  object->size.width  = obstacle.width();
  object->size.height = obstacle.height();
  object->type        = static_cast<msf::base::ObjectType>(obstacle.type());
  object->sub_type = static_cast<msf::base::ObjectSubType>(obstacle.sub_type());

  if (obstacle.type_probs_size() == 6) {
    for (int i = 0; i < obstacle.type_probs_size(); i++) {
      object->type_probs.push_back(obstacle.type_probs(i).prob());
    }
  }
}

bool TransTools::PbsToObjectsOutput(
    const std::shared_ptr<const airos::perception::PerceptionObstacles>&
        fusion_output,
    msf::FusionOutput* msg) {
  if (msg == nullptr) return false;
  if (fusion_output->perception_obstacle_size() < 1) {
    return false;
  }
  for (int i = 0; i < fusion_output->perception_obstacle_size(); i++) {
    msf::FusionObject fusion_object;
    PbToObjectOutput(fusion_output->perception_obstacle(i), &fusion_object);
    msg->objects.push_back(fusion_object);
  }
  return true;
}

void TransTools::FillObjectPolygonFromBBox3D(PerceptionObstacle* object_ptr) {
  struct PolygoPoint {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
  };

  if (!object_ptr) {
    return;
  }
  const double length = object_ptr->length();
  const double width  = object_ptr->width();
  double hl           = length / 2;
  double hw           = width / 2;
  double cos_theta    = std::cos(object_ptr->theta());
  double sin_theta    = std::sin(object_ptr->theta());

  // polygon.resize(4);
  PolygoPoint polygon[4];

  polygon[0].x = hl * cos_theta - hw * sin_theta + object_ptr->position().x();
  polygon[0].y = hl * sin_theta + hw * cos_theta + object_ptr->position().y();
  polygon[0].z = object_ptr->position().z();

  polygon[1].x = -hl * cos_theta - hw * sin_theta + object_ptr->position().x();
  polygon[1].y = -hl * sin_theta + hw * cos_theta + object_ptr->position().y();
  polygon[1].z = object_ptr->position().z();

  polygon[2].x = -hl * cos_theta + hw * sin_theta + object_ptr->position().x();
  polygon[2].y = -hl * sin_theta - hw * cos_theta + object_ptr->position().y();
  polygon[2].z = object_ptr->position().z();

  polygon[3].x = hl * cos_theta + hw * sin_theta + object_ptr->position().x();
  polygon[3].y = hl * sin_theta - hw * cos_theta + object_ptr->position().y();
  polygon[3].z = object_ptr->position().z();
  for (PolygoPoint point : polygon) {
    auto polygon_point = object_ptr->add_polygon_point();
    polygon_point->set_x(point.x);
    polygon_point->set_y(point.y);
    polygon_point->set_z(point.z);
  }
}

void TransTools::ObjectToPb(
    const msf::FusionObject& object, PerceptionObstacle* obstacle) {
  // times
  obstacle->set_timestamp(object.timestamp);
  // id
  obstacle->set_id(object.track_id);

  // tracking_time
  obstacle->set_tracking_time(object.tracking_time);
  // position
  obstacle->mutable_position()->set_x(object.center(0));  // center: Vector3d
  obstacle->mutable_position()->set_y(object.center(1));
  obstacle->mutable_position()->set_z(object.center(2));

  // yaw
  obstacle->set_theta(object.theta);  // flaot => double
  // lwh
  obstacle->set_length(object.size.length);
  obstacle->set_width(object.size.width);
  obstacle->set_height(object.size.height);
  FillObjectPolygonFromBBox3D(obstacle);

  // type
  obstacle->set_type(static_cast<PerceptionObstacle::Type>(object.type));
  obstacle->set_sub_type(static_cast<SubType>(object.sub_type));

  // type probs
  if (object.type_probs.size() == 6) {
    for (int i = 0; i < object.type_probs.size(); i++) {
      auto* type_prob = obstacle->add_type_probs();
      type_prob->set_type(static_cast<PerceptionObstacle::Type>(i));
      type_prob->set_prob(object.type_probs.at(i));
    }
  }
}

bool TransTools::PbsToObjects(
    const airos::perception::PerceptionObstacles& obstacles,
    msf::FusionInput* msg) {
  auto& objects = msg->objects;
  objects.clear();
  std::set<int> indexs;
  std::string frame_id = "";
  if (obstacles.has_header()) {
    frame_id = obstacles.header().frame_id();  // TODO fgq
  }

  for (int i = 0; i < obstacles.perception_obstacle_size(); ++i) {
    const PerceptionObstacle& obstacle = obstacles.perception_obstacle(i);
    if (obstacle.sub_type() == airos::perception::SubType::BARRIER_DELINEATOR ||
        obstacle.sub_type() == airos::perception::SubType::BARRIER_WATER ||
        indexs.find(i) != indexs.end()) {
      continue;
    }

    objects.emplace_back();
    msf::PerceptionObject& object = objects.back();
    PbToObject(obstacle, frame_id, &object);
  }

  msg->timestamp = obstacles.header().camera_timestamp() / 1e9;
  return true;
}

void TransTools::ObjectsToPbs(
    const msf::FusionOutput& message,
    airos::perception::PerceptionObstacles* obstacles) {
  obstacles->mutable_perception_obstacle()->Clear();
  if (message.objects.size() < 1) {
    return;
  }

  // obstacles->mutable_header()->set_frame_id(objects[0].frame_id);
  for (const auto& object : message.objects) {
    PerceptionObstacle obstacle;
    ObjectToPb(object, &obstacle);
    obstacles->add_perception_obstacle()->CopyFrom(obstacle);
  }
}

}  // namespace tools
}  // namespace visualization
}  // namespace perception
}  // namespace airos
