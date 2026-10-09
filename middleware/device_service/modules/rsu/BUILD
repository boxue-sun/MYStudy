load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "librsu_device",
    srcs = ["librsu_device.so"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "librsu_device.so",
    srcs = glob([
        "standard_rsu/protocol/*",
        "standard_rsu/*",
        "example/*",
    ]),
    linkshared = True,
    linkstatic = False,
    copts = [
        "-DMODULE_NAME=\\\"rsu_service\\\"",
    ],
    deps = [
        "//base",
        "@local//:glog",
        "@local//:yaml-cpp",
    ],
)

cc_test(
    name = "rsu_ut",
    srcs = glob([
        "standard_rsu/protocol/*.cc",
        "standard_rsu/*.cc",
        "standard_rsu/protocol/*.h",
        "standard_rsu/*.h",
        "example/*.cc",
        "example/*.h",
        "ut/rsu_test.cc",
    ]),
    deps = [
        "//base",
        "@com_google_googletest//:gtest_main",
        "@local//:glog",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
)
