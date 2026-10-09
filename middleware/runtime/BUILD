load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

cc_library(
    name = "airosrt",
    hdrs = glob(["src/*.h"]),
    visibility = ["//visibility:public"],
    deps = ["@cyber"],
)

cc_so_proto_library(
    name = "runtime_test_cc_pb",
    srcs = [":runtime_test_pb"],
)

proto_library(
    name = "runtime_test_pb",
    srcs = ["demo/test.proto"],
)

cc_binary(
    name = "runtime_component_demo",
    srcs = glob([
        "demo/component_demo.cc",
        "src/*.h",
    ]),
    deps = [
        "runtime_test_cc_pb",
        "//middleware/runtime:airosrt",
        "@com_google_googletest//:gtest_main",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
    ],
)

cc_binary(
    name = "runtime_node_demo",
    srcs = glob([
        "demo/node_demo.cc",
        "src/*.h",
    ]),
    deps = [
        "runtime_test_cc_pb",
        "//middleware/runtime:airosrt",
        "@com_google_googletest//:gtest_main",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
    ],
)

cc_binary(
    name = "runtime_listener_demo",
    srcs = glob([
        "demo/node_listener_demo.cc",
        "src/*.h",
    ]),
    deps = [
        "runtime_test_cc_pb",
        "//middleware/runtime:airosrt",
        "@com_google_googletest//:gtest_main",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
    ],
)

cc_binary(
    name = "runtime_talker_demo",
    srcs = glob([
        "demo/node_talker_demo.cc",
        "src/*.h",
    ]),
    deps = [
        "runtime_test_cc_pb",
        "//middleware/runtime:airosrt",
        "@com_google_googletest//:gtest_main",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
    ],
)

cc_test(
    name = "runtime_ut",
    srcs = glob([
        "ut/runtime_test.cc",
        "src/*.h",
    ]),
    deps = [
        "//middleware/runtime:airosrt",
        "@com_google_googletest//:gtest_main",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
    ],
)
