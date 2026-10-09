#include "example_perception_fusion.h"

namespace airos {
namespace perception {
namespace fusion {

bool ExamplePerceptionFusion::Init(const PerceptionFusionParam &param) {
  std::cout << "Init: " << param.alg_name() << std::endl;
  std::cout << "alg_path: " << param.alg_path() << std::endl;
  /*
    init param
  */
  return true;
}

bool ExamplePerceptionFusion::Process(
    const std::vector<std::shared_ptr<const PerceptionObstacles>> &input,
    PerceptionObstacles *result) {
  /*
    fusion algrithm
  */
  return true;
}

}  // namespace fusion
}  // namespace perception
}  // namespace airos
