load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//visibility:public"])

cc_so_proto_library(
    name = "omt_cc_pb",
    srcs = [":omt_pb"],
)

proto_library(
    name = "omt_pb",
    srcs = ["air_tracker/tracker/omt.proto"],
)

cc_library(
    name = "tracker_plugin_adapter",
    srcs = glob([
        "*.cc",
        "air_tracker/*.cc",
        "air_tracker/common_operator/*.cc",
        "air_tracker/track_common/*.cc",
        "air_tracker/tracker/*.cc",
        "air_tracker/common_operator/munkres/*.cc",
    ]),
    hdrs = glob([
        "*.h",
        "air_tracker/*.h",
        "air_tracker/common_operator/*.h",
        "air_tracker/track_common/*.h",
        "air_tracker/tracker/*.h",
        "air_tracker/common_operator/munkres/*.h",
    ]),
    copts = [
        "-Iair_service/modules/perception-camera/algorithm/tracker",
    ],
    deps = [
        "omt_cc_pb",
        "//air_service/modules/perception-camera/algorithm/interface:camera_algorithm_interface",
        "//air_service/modules/perception-camera/proto:track_param_cc_pb",
        "//base",
        "@local//:opencv",
    ],
)
