load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "libtraffic_light_device",
    srcs = ["libtraffic_light_device.so"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "libtraffic_light_device.so",
    srcs = glob([
        "example/*",
        "gat1743/*",
    ]),
    linkshared = True,
    linkstatic = False,
    copts = [
        "-DMODULE_NAME=\\\"traffic_light_service\\\"",
    ],
    deps = [
        "//base",
        "@local//:glog",
        "@local//:yaml-cpp",
    ],
)

cc_test(
    name = "traffic_light_ut",
    srcs = glob([
        "example/*.cc",
        "example/*.h",
        "gat1743/*.cc",
        "gat1743/*.h",
        "ut/traffic_light_test.cc",
    ]),
    deps = [
        "//base",
        "@com_google_googletest//:gtest_main",
        "@local//:glog",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
    linkopts = ["-lrt"],
)

cc_binary(
    name = "traffic_light_test_tool",
    srcs = glob([
        "example/*.cc",
        "example/*.h",
        "gat1743/*.cc",
        "gat1743/*.h",
        "test_tool/traffic_light_gat1743_test_tool.cc",
    ]),
    deps = [
        "//base",
        "@local//:glog",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
    linkopts = ["-lrt", "-lpthread"],
)
