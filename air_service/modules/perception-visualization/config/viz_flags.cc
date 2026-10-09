
#include "air_service/modules/perception-visualization/config/viz_flags.h"

namespace airos {
namespace perception {
namespace visualization {

// DEFINE_bool(obs_enable_visualization, false,
//   "whether to send message for visualization");
// DEFINE_string(obs_screen_output_dir, "./",
//   "output dir. for saving visualization screenshots");
// DEFINE_bool(obs_benchmark_mode, false,
//   "whether open benchmark mode, default false");
// DEFINE_bool(obs_save_fusion_supplement, false,
//   "whether save fusion supplement data, default false");

// DEFINE_string(visualizer_conf_model_name,
// "GLFusionVisualizer_camera_onboard", "the model name of visulizer");
// DEFINE_string(main_camera_name, "camera1", "the name of main camera");
DEFINE_string(
    viz_config_path,
    "/home/airos/os/conf/viz/viz_config.pt",
    "path of viewer config file");
// DEFINE_string(display_ini_option_path, "display_options.ini", "path of
// display options' init config file");
}  // namespace visualization
}  // namespace perception
}  // namespace airos
