load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//visibility:public"])

cc_so_proto_library(
    name = "msf_config_cc_pb",
    srcs = [":msf_config_pb"],
)

proto_library(
    name = "msf_config_pb",
    srcs = [
        "fusion_params.proto",
        "multi_sensor_fusion_config.proto",
    ],
)
