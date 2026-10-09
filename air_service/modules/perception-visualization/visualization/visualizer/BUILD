load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "perception_visualization_visualizer",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    deps = [
        "//air_service/modules/perception-visualization/config:perception_visualization_config",
        "//air_service/modules/perception-visualization/visualization/base:perception_visualization_base",
        "//air_service/modules/perception-visualization/visualization/common:perception_visualization_common",
        "//air_service/modules/perception-visualization/visualization/viewports:perception_visualization_viewports",
        "//base",
        "@eigen",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glew",
        "@local//:glfw",
        "@local//:glog",
        "@local//:jsoncpp",
        "@local//:opencv",
        "@local//:yaml-cpp",
    ],
)
