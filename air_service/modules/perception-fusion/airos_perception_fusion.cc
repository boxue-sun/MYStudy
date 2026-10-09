#include "air_service/modules/perception-fusion/airos_perception_fusion.h"

#include <future>

#include "air_service/modules/perception-fusion/algorithm/air_fusion/air_object_fusion.h"
#include "base/common/time_util.h"
#include "base/io/file_util.h"
#include "base/io/protobuf_util.h"
#include "msf_trans_tools.h"

namespace airos {
namespace perception {
namespace msf {

void AirosPerceptionFusion::SerializeMsg(
    const msf::FusionOutput &message, PerceptionObstacles *output_msg,
    int error_code) {
  tools::TransTools::ObjectsToPbs(message, output_msg);
  airos::header::Header *header = output_msg->mutable_header();
  header->set_timestamp_sec(airos::base::TimeUtil::GetCurrentTime());
  header->set_module_name("v2x_fusion");
  header->set_sequence_num(seq_num_);

  if (error_code == static_cast<int>(msf::base::Error::NONE)) {
    long timestamp = long(air_fusion_->Timestamp() * 1e9);
    header->set_camera_timestamp(timestamp);
    output_msg->set_error_code(
        airos::perception::PerceptionErrorCode::ERROR_NONE);
  } else {
    output_msg->set_error_code(
        airos::perception::PerceptionErrorCode::ERROR_PROCESS);
  }
}

bool AirosPerceptionFusion::Init(const PerceptionFusionParam &param) {
  std::string param_path = param.alg_path() + "/conf/msf_config.pt";
  if (!airos::base::FileUtil::Exists(param_path)) return false;
  if (airos::base::ParseProtobufFromFile<MsfConfig>(param_path, &air_config_) ==
      false) {
    return false;
  }
  param_ = param;

  BaseMultiSensorFusion::InitParam sdk_param;
  sdk_param.channel_num = param_.input_size();
  sdk_param.time_diff   = air_config_.timestamp_param().time_diff();
  sdk_param.missing_time_threshold =
      air_config_.timestamp_param().missing_time_threshold();
  sdk_param.reverse_time_threshold =
      air_config_.timestamp_param().reverse_time_threshold();
  sdk_param.collision_line_check = air_config_.collision_line_check();
  for (int i = 0; i < air_config_.timestamp_param().reference_seq_size(); ++i) {
    sdk_param.reference_seq.insert(
        air_config_.timestamp_param().reference_seq(i));
  }
  sdk_param.fusion_params_file = param.alg_path() + "/conf/fusion_params.pt";
  if (!airos::base::FileUtil::Exists(sdk_param.fusion_params_file))
    return false;

  air_fusion_ = std::make_shared<AirObjectFusion>();
  if (air_fusion_->Init(sdk_param) == false) {
    LOG_ERROR << "fusion init failed.";
    return false;
  }
  return true;
}

bool AirosPerceptionFusion::Process(
    const std::vector<std::shared_ptr<const PerceptionObstacles>> &input,
    PerceptionObstacles *result) {
  if (result == nullptr) return false;
  if (input.size() != param_.input_size()) return false;

  std::vector<msf::FusionInput> sdk_input(input.size());

  double start_time = airos::base::TimeUtil::GetCurrentTime();
  std::vector<std::future<void>> ret_futures;
  auto post_task = [&, this](int i) {
    if (input[i] != nullptr)
      tools::TransTools::PbsToObjects(*(input[i]), &sdk_input[i]);
    sdk_input[i].index = i;
    sdk_input[i].collision_line_fusion =
        air_config_.input(i).collision_line_fusion();
  };
  for (int i = 0; i < input.size(); ++i) {
    ret_futures.emplace_back(std::async(std::launch::async, post_task, i));
  }
  for (auto &ret_future : ret_futures) {
    ret_future.get();
  }
  double finish_time = airos::base::TimeUtil::GetCurrentTime();
  LOG_INFO << "AirosPerceptionFusion PbsToObjects cost "
           << static_cast<int>((finish_time - start_time) * 1000) << " ms";

  msf::FusionOutput sdk_output;
  int error_code = air_fusion_->Process(sdk_input, sdk_output);

  start_time = airos::base::TimeUtil::GetCurrentTime();
  SerializeMsg(sdk_output, result, error_code);
  finish_time = airos::base::TimeUtil::GetCurrentTime();
  LOG_INFO << "AirosPerceptionFusion SerializeMsg cost "
           << static_cast<int>((finish_time - start_time) * 1000) << " ms";
  return true;
}

}  // namespace msf
}  // namespace perception
}  // namespace airos
