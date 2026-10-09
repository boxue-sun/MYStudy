load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "roi_filter_adapter",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    deps = [
        "//air_service/modules/perception-camera/algorithm/interface:camera_algorithm_interface",
        "//air_service/modules/perception-camera/proto:roi_filter_param_cc_pb",
        "//base",
        "@local//:glog",
        "@local//:opencv",
    ],
)
