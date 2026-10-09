load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "perception_visualization_base",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    deps = [
        "//air_service/modules/perception-camera/algorithm/interface:camera_algorithm_interface",
        "//base",
        "@eigen",
        "@local//:boost",
        "@local//:glew",
        "@local//:glog",
    ],
)
