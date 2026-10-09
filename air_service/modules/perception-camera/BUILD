load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libairos_perception_camera.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    copts = [
        "-DMODULE_NAME=\\\"perception-camera\\\"",
    ],
    linkshared = True,
    linkstatic = False,
    visibility = ["//visibility:public"],
    deps = [
        "//air_service/framework/perception-camera/interface:base_perception_interface",
        "//air_service/framework/proto:airos_service_pb",
        "//air_service/modules/perception-camera/algorithm/detector:detector_plugin_adapter",
        "//air_service/modules/perception-camera/algorithm/interface:camera_algorithm_interface",
        "//air_service/modules/perception-camera/algorithm/roi_filter:roi_filter_adapter",
        "//air_service/modules/perception-camera/algorithm/tracker:tracker_plugin_adapter",
        "//air_service/modules/perception-camera/algorithm/transformer:transformer_plugin_adapter",
        "//air_service/modules/perception-camera/proto:perception_param_cc_pb",
        "//base",
        "@local//:atlas",
        "@local//:boost",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
    ],
)

cc_library(
    name = "airos_perception_camera",
    srcs = ["libairos_perception_camera.so"],
    hdrs = ["airos_camera_perception.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//air_service/framework/perception-camera/interface:base_perception_interface",
        "//air_service/framework/proto:airos_service_pb",
        "//air_service/modules/perception-camera/algorithm/detector:detector_plugin_adapter",
        "//air_service/modules/perception-camera/algorithm/interface:camera_algorithm_interface",
        "//air_service/modules/perception-camera/algorithm/roi_filter:roi_filter_adapter",
        "//air_service/modules/perception-camera/algorithm/tracker:tracker_plugin_adapter",
        "//air_service/modules/perception-camera/algorithm/transformer:transformer_plugin_adapter",
        "//air_service/modules/perception-camera/proto:perception_param_cc_pb",
        "//base",
        "@local//:atlas",
        "@local//:boost",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
    ],
)
