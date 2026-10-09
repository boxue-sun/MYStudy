load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libperception_adapter.so",
    linkshared = True,
    linkstatic = False,
    deps = [":perception_adapter_lib"],
)

cc_library(
    name = "perception_adapter_lib",
    srcs = ["perception_adapter.cc"],
    hdrs = ["perception_adapter.h"],
    copts = [
        "-DMODULE_NAME=\\\"perception_adapter\\\"",
    ],
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//app/modules/v2x_application/common:app_common",
        "//base",
        "@local//:gflags",
        "@local//:glog",
    ],
)

cc_test(
    name = "perception_adapter_ut",
    srcs = [
        "perception_adapter.h",
        "ut/perception_adapter_test.cc",
    ],
    linkstatic = False,
    deps = [
        ":perception_adapter_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
