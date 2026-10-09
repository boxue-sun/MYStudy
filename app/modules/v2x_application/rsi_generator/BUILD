load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "librsi_generator.so",
    linkshared = True,
    linkstatic = False,
    deps = [":rsi_generator_lib"],
)

cc_library(
    name = "rsi_generator_lib",
    srcs = glob([
        "rsi_generator.cc",
        "affect_path/*.cc",
    ]),
    hdrs = glob([
        "rsi_generator.h",
        "affect_path/*.h",
    ]),
    linkopts = [
        "-lGeographic"
    ],
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//app/framework/interface:app_interface",
        "//app/framework/proto:airos_app_pb",
        "//app/modules/v2x_application/common:app_common",
        "//app/modules/v2x_application/proto:v2xpb-config-event-details_cc_pb",
        "//app/modules/v2x_application/proto:v2xpb-rsi-config_cc_pb",
        "//app/modules/v2x_application/usecase/common:math",
        "//base",
        "@asn_wrapper",
        "@local//:gflags",
        "@local//:glog",
    ],
)

cc_test(
    name = "rsi_generator_ut",
    srcs = [
        "rsi_generator.h",
        "ut/rsi_generator_test.cc",
    ],
    linkstatic = False,
    deps = [
        ":rsi_generator_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
