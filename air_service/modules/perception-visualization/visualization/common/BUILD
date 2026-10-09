load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "perception_visualization_common",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    #"//air_service/modules/perception-visualization/visualization/visualizer:perception_visualization_visualizer",
    #"//air_service/modules/perception-visualization/visualization/viewports:perception_visualization_viewports",
    deps = [
        "//air_service/modules/perception-camera/algorithm/interface:camera_algorithm_interface",
        "//air_service/modules/perception-visualization/visualization/base:perception_visualization_base",
        "//base",
        "@eigen",
        "@local//:boost",
        "@local//:glew",
        "@local//:glfw",
        "@local//:glog",
        "@local//:opengl",
    ],
)
