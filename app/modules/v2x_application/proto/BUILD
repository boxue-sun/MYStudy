load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//visibility:public"])

cc_so_proto_library(
    name = "v2xpb-config-event-details_cc_pb",
    srcs = [":v2xpb-config-event-details_pb"],
)

proto_library(
    name = "v2xpb-config-event-details_pb",
    srcs = ["v2xpb-config-event-details.proto"],
)

cc_so_proto_library(
    name = "v2xpb-rsi-config_cc_pb",
    srcs = [":v2xpb-rsi-config_pb"],
)

proto_library(
    name = "v2xpb-rsi-config_pb",
    srcs = ["v2xpb-rsi-config.proto"],
)

cc_proto_library(
    name = "roi_lane_cc_pb",
    deps = [":roi_lane_pb"],
)

proto_library(
    name = "roi_lane_pb",
    srcs = [
        "roi_lane.proto",
        "roi_points.proto",
    ],
)

cc_so_proto_library(
    name = "rscu_status_cc_pb",
    srcs = [":rscu_status_pb"],
)

proto_library(
    name = "rscu_status_pb",
    srcs = ["rscu_status.proto"],
)
