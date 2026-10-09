load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libairos_perception_fusion.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    copts = [
        "-DMODULE_NAME=\\\"perception-fusion\\\"",
    ],
    linkshared = True,
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        "//air_service/framework/perception-fusion/interface:base_fusion_interface",
        "//air_service/framework/perception-fusion/proto:fusion_config_cc_pb",
        "//air_service/modules/perception-fusion/algorithm/air_fusion:air_object_fusion",
        "//air_service/modules/perception-fusion/algorithm/interface:multi_sensor_fusion",
        "//air_service/modules/perception-fusion/proto:msf_config_cc_pb",
        "//base",
        "@local//:glog",
        "@local//:jsoncpp",
        "@local//:opencv",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
)

cc_library(
    name = "airos_perception_fusion",
    srcs = ["libairos_perception_fusion.so"],
    hdrs = ["airos_perception_fusion.h"],
    deps = [
        "//air_service/framework/perception-fusion/interface:base_fusion_interface",
        "//air_service/framework/perception-fusion/proto:fusion_config_cc_pb",
        "//base",
        "@local//:gflags",
        "@local//:glog",
        "@local//:jsoncpp",
        "@local//:opencv",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
)
