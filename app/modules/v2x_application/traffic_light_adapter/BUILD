load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libtraffic_light_adapter.so",
    linkshared = True,
    linkstatic = False,
    deps = [":traffic_light_adapter_lib"],
)

cc_library(
    name = "traffic_light_adapter_lib",
    srcs = ["traffic_light_adapter.cc"],
    hdrs = ["traffic_light_adapter.h"],
    deps = [
        "//middleware/runtime:airosrt",
        "//air_service/framework/proto:airos_service_pb",
        "//app/framework/interface:app_interface",
        "//app/framework/proto:airos_app_pb",
        "//app/modules/v2x_application/common:app_common",
        "//base",
        "@local//:gflags",
        "@local//:glog",
        "@local//:fastrtps",
         "//middleware/protocol/proto:monitor_mec_pb",
    ],
)

cc_test(
    name = "traffic_light_adapter_ut",
    srcs = [
        "traffic_light_adapter.h",
        "ut/traffic_light_adapter_test.cc",
    ],
    linkstatic = False,
    deps = [
        ":traffic_light_adapter_lib",
        "@com_google_googletest//:gtest_main",
        "@local//:yaml-cpp",

    ],
)
