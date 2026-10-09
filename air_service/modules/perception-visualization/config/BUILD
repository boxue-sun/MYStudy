load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "perception_visualization_config",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    deps = [
        "viz_config_cc_pb",
        "//base",
        "@eigen",
        "@local//:boost",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
    ],
)

cc_so_proto_library(
    name = "viz_config_cc_pb",
    srcs = [":viz_config_pb"],
)

proto_library(
    name = "viz_config_pb",
    srcs = ["viz_config.proto"],
)
