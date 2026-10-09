load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//visibility:public"])

proto_library(
    name = "traffic_light_config_pb",
    srcs = ["traffic_light_config.proto"],
)

cc_so_proto_library(
    name = "traffic_light_config_cc_pb",
    srcs = [":traffic_light_config_pb"],
)

proto_library(
    name = "rsu_config_pb",
    srcs = ["rsu_config.proto"],
)

cc_so_proto_library(
    name = "rsu_config_cc_pb",
    srcs = [":rsu_config_pb"],
)

proto_library(
    name = "cloud_config_pb",
    srcs = ["cloud_config.proto"],
)

cc_so_proto_library(
    name = "cloud_config_cc_pb",
    srcs = [":cloud_config_pb"],
)

proto_library(
    name = "mec_config_pb",
    srcs = ["mec_config.proto"],
)

cc_so_proto_library(
    name = "mec_config_cc_pb",
    srcs = [":mec_config_pb"],
)


proto_library(
    name = "camera_cictci_config_pb",
    srcs = ["camera_cictci_config.proto"],
)

cc_so_proto_library(
    name = "camera_cictci_config_cc_pb",
    srcs = [":camera_cictci_config_pb"],
)

proto_library(
    name = "ipcamera_config_pb",
    srcs = ["ipcamera_config.proto"],
)

cc_so_proto_library(
    name = "ipcamera_config_cc_pb",
    srcs = [":ipcamera_config_pb"],
)
