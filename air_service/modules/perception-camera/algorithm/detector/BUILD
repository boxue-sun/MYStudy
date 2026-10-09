load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_cuda//cuda:defs.bzl", "cuda_library")

package(default_visibility = ["//visibility:public"])

cuda_library(
    name = "gpu",
    srcs = [
        "air_detector/gpu.cu",
    ],
    hdrs = [
        "air_detector/gpu.h",
    ],
    deps = [
        "//base",
        "@cuda",
        "@local//:glog",
        "@local//:opencv",
    ],
)

cc_library(
    name = "detector_plugin_adapter",
    srcs = glob([
        "*.cc",
        "**/*.cc",
    ]),
    hdrs = glob([
        "*.h",
        "**/*.h",
    ]),
    deps = [
        "gpu",
        "//air_service/modules/perception-camera/algorithm/interface:camera_algorithm_interface",
        "//air_service/modules/perception-camera/proto:detector_param_cc_pb",
        "//base",
        "@cuda",
        "@local//:tensorrt",
        "@local//:yaml-cpp",
        "@paddle",
    ],
)
