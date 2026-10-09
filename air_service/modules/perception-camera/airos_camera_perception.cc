#include "airos_camera_perception.h"

#include "base/common/time_util.h"
#include "base/io/file_util.h"
#include "base/io/protobuf_util.h"

namespace airos {
namespace perception {
namespace camera {

static void SerializeMsg(PerceptionFrame& frame, PerceptionObstacles& out_msg) {
  auto header         = out_msg.mutable_header();
  double current_time = airos::base::TimeUtil::GetCurrentTime();
  header->set_timestamp_sec(current_time);              // second
  header->set_frame_id(frame.camera_name);              // TODO
  header->set_camera_timestamp(frame.timestamp * 1e9);  // nano-second
  // // frame.camera_name;
  for (auto& object : frame.objects) {
    auto obstacle = out_msg.add_perception_obstacle();
    obstacle->set_timestamp(frame.timestamp);
    obstacle->set_id(object.track_id);
    // frame.type_id_confidence
    auto box = obstacle->mutable_bbox2d();
    box->set_xmin(object.box.left_top.x);
    box->set_ymin(object.box.left_top.y);
    box->set_xmax(object.box.right_bottom.x);
    box->set_ymax(object.box.right_bottom.y);
    obstacle->set_length(object.size.length);
    obstacle->set_width(object.size.width);
    obstacle->set_height(object.size.height);
    obstacle->set_theta(object.theta);
    auto position = obstacle->mutable_position();
    position->set_x(object.center(0));
    position->set_y(object.center(1));
    position->set_z(object.center(2));
    switch (object.is_occluded) {
      case algorithm::TriStatus::UNKNOWN:
        obstacle->set_occ_state(OcclusionState::OCC_UNKNOWN);
        break;
      case algorithm::TriStatus::TRUE:
        obstacle->set_occ_state(OcclusionState::OCC_COMPLETE_OCCLUDED);
        break;
      case algorithm::TriStatus::FALSE:
        obstacle->set_occ_state(OcclusionState::OCC_NONE_OCCLUDED);
        break;
      default:
        break;
    }
    switch (object.is_truncated) {
      case algorithm::TriStatus::UNKNOWN:
        obstacle->set_trunc_state(TruncationState::TRUNC_UNKNOWN);
        break;
      case algorithm::TriStatus::TRUE:
        obstacle->set_trunc_state(TruncationState::TRUNC_TRUE);
        break;
      case algorithm::TriStatus::FALSE:
        obstacle->set_trunc_state(TruncationState::TRUNC_FALSE);
        break;
      default:
        break;
    }
    obstacle->set_theta_variance(object.theta_variance);
    for (auto& pt : object.pts8.pts8) {
      auto point = obstacle->add_cube_pts8();
      point->set_x(pt.x);
      point->set_y(pt.y);
    }

    auto direction = obstacle->mutable_cube_pts8(0);
    direction->set_x(object.direction.x);
    direction->set_y(object.direction.y);
    direction->set_z(object.direction.z);
    auto center = obstacle->mutable_cube_pts8(1);
    center->set_x(object.center[0]);
    center->set_y(object.center[1]);
    center->set_z(object.center[2]);

    obstacle->set_sub_type(static_cast<perception::SubType>(object.type));
    for (int i = 0; i < object.sub_type_probs.size(); ++i) {
      auto sub_type_prob = obstacle->add_sub_type_probs();
      auto sub_type      = static_cast<perception::SubType>(i);
      sub_type_prob->set_sub_type(sub_type);
      sub_type_prob->set_prob(object.sub_type_probs[i]);
    }
    obstacle->set_sub_type_id_confidence(object.type_id_confidence);
    obstacle->set_sub_type_id(object.type_id);
  }
}

bool AirosCameraPerception::Init(const CameraPerceptionParam& param) {
  AirPerceptionParam alg_param;
  std::string alg_conf_file = param.sdk_path + "/conf/" + param.camera_name +
                              "/" + param.camera_name + ".pt";
  if (airos::base::ParseProtobufFromFile<AirPerceptionParam>(
          alg_conf_file, &alg_param) == false) {
    return false;
  }

  std::vector<airos::base::PluginParam> plugin_params;
  for (int i = 0; i < alg_param.plugin_param_size(); ++i) {
    airos::base::PluginParam plugin_param;
    auto& proto_param        = alg_param.plugin_param(i);
    plugin_param.name        = proto_param.name();
    plugin_param.config_file = param.sdk_path + "/" + proto_param.root_dir() +
                               "/" + proto_param.config_file();
    plugin_params.emplace_back(std::move(plugin_param));
  }
  return plugin_manager_.Init(plugin_params);
}

bool AirosCameraPerception::Perception(
    const base::device::CameraImageData& camera_data,
    PerceptionObstacles* result, TrafficLightDetection* traffic_light) {
  if (!result || !traffic_light) return false;
  PerceptionFrame perception_frame;
  perception_frame.image       = camera_data.image;
  perception_frame.camera_name = camera_data.camera_name;
  perception_frame.timestamp   = camera_data.timestamp * 1e-6;
  perception_frame.frame_id    = camera_data.sequence_num;
  if (plugin_manager_.Run(perception_frame) == false) return false;
  SerializeMsg(perception_frame, *result);
  return true;
}

}  // namespace camera
}  // namespace perception
}  // namespace airos
