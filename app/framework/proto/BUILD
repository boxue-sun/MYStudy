load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//visibility:public"])

cc_so_proto_library(
    name = "airos_app_pb",
    srcs = [":airos_app_pb_lib"],
)

proto_library(
    name = "airos_app_pb_lib",
    srcs = glob(["*.proto"]),
)
