#pragma once

#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "air_service/modules/perception-camera/proto/perception_param.pb.h"

#include "air_service/framework/perception-camera/interface/base_camera_perception.h"
#include "air_service/modules/perception-camera/algorithm/detector/detector_plugin_adapter.h"
#include "air_service/modules/perception-camera/algorithm/interface/perception_frame.h"
#include "air_service/modules/perception-camera/algorithm/roi_filter/roi_judgement_adapter.h"
#include "air_service/modules/perception-camera/algorithm/tracker/tracker_plugin_adapter.h"
#include "air_service/modules/perception-camera/algorithm/transformer/transformer_plugin_adapter.h"
#include "base/plugin/algorithm_plugin.h"

namespace airos {
namespace perception {
namespace camera {

class AirosCameraPerception : public BaseCameraPerception {
 public:
  AirosCameraPerception()                                         = default;
  AirosCameraPerception(const AirosCameraPerception &)            = delete;
  AirosCameraPerception &operator=(const AirosCameraPerception &) = delete;
  ~AirosCameraPerception()                                        = default;

  bool Init(const CameraPerceptionParam &param);
  bool Perception(
      const base::device::CameraImageData &camera_data,
      PerceptionObstacles *result, TrafficLightDetection *traffic_light);
  std::string Name() const override {
    return "AirosCameraPerception";
  }

 private:
  airos::base::PluginManager<PerceptionFrame> plugin_manager_;
};

REGISTER_ALGORITHM_PLUGIN_ELEMENT(DetectorPluginAdapter, PerceptionFrame);
REGISTER_ALGORITHM_PLUGIN_ELEMENT(RoiPluginAdapter, PerceptionFrame);
REGISTER_ALGORITHM_PLUGIN_ELEMENT(TrackerPluginAdapter, PerceptionFrame);
REGISTER_ALGORITHM_PLUGIN_ELEMENT(TransformerPluginAdapter, PerceptionFrame);

REGISTER_CAMERA_PERCEPTION(AirosCameraPerception);

}  // namespace camera
}  // namespace perception
}  // namespace airos
