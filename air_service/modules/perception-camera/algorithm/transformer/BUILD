load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "transformer_plugin_adapter",
    srcs = glob([
        "*.cc",
        "**/*.cc",
    ]),
    hdrs = glob([
        "*.h",
        "**/*.h",
    ]),
    deps = [
        "//air_service/modules/perception-camera/algorithm/interface:camera_algorithm_interface",
        "//air_service/modules/perception-camera/proto:transform_param_cc_pb",
        "//base",
        "//middleware/device_service/framework/camera:mid_camera_service",
    ],
)
