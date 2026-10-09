load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libmonitor.so",
    linkshared = True,
    linkstatic = False,
    deps = [":monitor_lib"],
)

cc_library(
    name = "monitor_lib",
    srcs = [
        "common/system_cmd.cc",
        "hardware/device_monitor.cc",
        "monitor.cc",
    ],
    hdrs = [
        "common/system_cmd.h",
        "hardware/device_monitor.h",
        "monitor.h",
    ],
    deps = [
        "//app/modules/v2x_application/common:app_common",
        "//app/modules/v2x_application/proto:rscu_status_cc_pb",
        "//base",
        "@local//:gflags",
        "@local//:glog",
        "@local//:yaml-cpp",
    ],
)

cc_test(
    name = "monitor_ut",
    srcs = [
        "monitor.h",
        "ut/monitor_test.cc",
    ],
    linkstatic = False,
    deps = [
        ":monitor_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
