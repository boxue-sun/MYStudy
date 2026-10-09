load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//visibility:public"])

cc_so_proto_library(
    name = "airos_service_pb",
    srcs = [":perception_pb"],
)

proto_library(
    name = "perception_pb",
    srcs = [
        "airos_header.proto",
        "airos_perception_common.proto",
        "airos_perception_obstacle.proto",
        "airos_traffic_light.proto",
        "airos_usecase.proto",
        "airos_visual_traffic_light.proto",
    ],
)
