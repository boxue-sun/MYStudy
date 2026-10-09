load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//base:__subpackages__"])

cc_so_proto_library(
    name = "airos_device_pb",
    srcs = [":airos_device_pb_lib"],
)

proto_library(
    name = "airos_device_pb_lib",
    srcs = [
        "cloud_data.proto",
        "ins_data.proto",
        "lidar_data.proto",
        "radar_data.proto",
        "rsu_data.proto",
        "traffic_light_data.proto",
        "ipcamera.proto",
    ],
)
