load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libv2x_message_reporter.so",
    linkshared = True,
    linkstatic = False,
    deps = [":v2x_message_reporter_lib"],
)

cc_library(
    name = "v2x_message_reporter_lib",
    srcs = ["v2x_message_reporter.cc"],
    hdrs = ["v2x_message_reporter.h"],
    deps = [
        "//app/framework/proto:airos_app_pb",
        "//app/modules/v2x_application/common:app_common",
        "//base",
        "@local//:gflags",
        "@local//:glog",
    ],
)

cc_test(
    name = "v2x_message_reporter_ut",
    srcs = [
        "ut/v2x_message_reporter_test.cc",
        "v2x_message_reporter.h",
    ],
    linkstatic = False,
    deps = [
        ":v2x_message_reporter_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
