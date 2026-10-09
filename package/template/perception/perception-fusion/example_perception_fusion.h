#pragma once

#include "air_service/framework/perception-fusion/interface/base_perception_fusion.h"

namespace airos {
namespace perception {
namespace fusion {

class ExamplePerceptionFusion : public BasePerceptionFusion {
 public:
  ExamplePerceptionFusion()  = default;
  ~ExamplePerceptionFusion() = default;
  bool Init(const PerceptionFusionParam &param) override;
  bool Process(
      const std::vector<std::shared_ptr<const PerceptionObstacles>> &input,
      PerceptionObstacles *result) override;
  virtual std::string Name() const override {
    return "ExamplePerceptionFusion";
  }
};

REGISTER_PERCEPTION_FUSION(ExamplePerceptionFusion);

}  // namespace fusion
}  // namespace perception
}  // namespace airos
