load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//visibility:public"])

cc_so_proto_library(
    name = "perception_camera_param_cc_pb",
    srcs = [":perception_camera_param_pb"],
)

proto_library(
    name = "perception_camera_param_pb",
    srcs = ["perception_camera.proto"],
)
