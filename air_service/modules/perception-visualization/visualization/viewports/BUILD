load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "perception_visualization_viewports",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    deps = [
        "//air_service/modules/perception-camera/algorithm/interface:camera_algorithm_interface",
        "//air_service/modules/perception-visualization/config:perception_visualization_config",
        "//air_service/modules/perception-visualization/visualization/base:perception_visualization_base",
        "//air_service/modules/perception-visualization/visualization/common:perception_visualization_common",
        "//base",
        "@eigen",
        "@local//:boost",
        "@local//:glew",
        "@local//:glfw",
        "@local//:glog",
    ],
)
