load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//visibility:public"])

cc_so_proto_library(
    name = "perception_param_cc_pb",
    srcs = [":perception_param_pb"],
)

proto_library(
    name = "perception_param_pb",
    srcs = ["perception_param.proto"],
)

cc_so_proto_library(
    name = "detector_param_cc_pb",
    srcs = [":detector_param_pb"],
)

proto_library(
    name = "detector_param_pb",
    srcs = ["detector_param.proto"],
)

cc_so_proto_library(
    name = "roi_filter_param_cc_pb",
    srcs = [":roi_filter_param_pb"],
)

proto_library(
    name = "roi_filter_param_pb",
    srcs = ["roi_filter_param.proto"],
)

cc_so_proto_library(
    name = "track_param_cc_pb",
    srcs = [":track_param_pb"],
)

proto_library(
    name = "track_param_pb",
    srcs = ["track_param.proto"],
)

cc_so_proto_library(
    name = "transform_param_cc_pb",
    srcs = [":transform_param_pb"],
)

proto_library(
    name = "transform_param_pb",
    srcs = ["transform_param.proto"],
)
