// Copyright 2018 Baidu.com, Inc. All Rights Reserved.
// @author: Cheng Kai(chengkai03@baidu.com)
// @file: common_flags.h
// @brief: common gflag configuration
#ifndef PERCEPTION_ONBOARD_COMMON_COMMON_FLAGS_H_
#define PERCEPTION_ONBOARD_COMMON_COMMON_FLAGS_H_

#include <gflags/gflags.h>

namespace airos {
namespace perception {
namespace visualization {

// DECLARE_bool(obs_enable_hdmap_input);
// DECLARE_bool(obs_enable_visualization);
// DECLARE_string(obs_screen_output_dir);
// DECLARE_bool(obs_benchmark_mode);
// DECLARE_bool(obs_save_fusion_supplement);

// DECLARE_string(visualizer_conf_model_name);
// DECLARE_string(main_camera_name);
DECLARE_string(viz_config_path);
// DECLARE_string(display_ini_option_path);
}  // namespace visualization
}  // namespace perception
}  // namespace airos

#endif  // PERCEPTION_ONBOARD_COMMON_INNER_COMPONENT_MESSAGE_H_
