load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libusecase_adapter.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkshared = True,
    linkstatic = False,
    visibility = ["//visibility:public"],
    deps = [
        "//app/modules/v2x_application/proto:roi_lane_cc_pb",
        "//app/modules/v2x_application/usecase/algorithm:lane_congestion",
        "//app/modules/v2x_application/usecase/algorithm:params",
        "//app/modules/v2x_application/usecase/base:config_manager",
        "//app/modules/v2x_application/usecase/base:runner",
        "//app/modules/v2x_application/usecase/common:factory",
        "//app/modules/v2x_application/usecase/common:math",
        "//app/modules/v2x_application/usecase/common:timer",
        "//app/modules/v2x_application/usecase/common:util",
        "//app/modules/v2x_application/usecase/map:map_info",
        "//base",
        "@local//:gflags",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
)

cc_library(
    name = "usecase_adapter",
    srcs = ["libusecase_adapter.so"],
    hdrs = ["usecase_adapter.h"],
    deps = [
        "//app/modules/v2x_application/common:app_common",
        "//app/modules/v2x_application/usecase/base:config_manager",
        "//app/modules/v2x_application/usecase/base:runner",
        "//app/modules/v2x_application/usecase/common:timer",
        "//app/modules/v2x_application/usecase/map:map_info",
        "//base",
        "@local//:gflags",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
)

cc_binary(
    name = "libusecase_adapter_viz.so",
    linkshared = True,
    linkstatic = False,
    deps = [":usecase_adapter_viz_lib"],
)

cc_library(
    name = "usecase_adapter_viz_lib",
    srcs = [
        "usecase_adapter.cc",
    ],
    hdrs = [
        "usecase_adapter.h",
    ],
    defines = ["USE_VIZ"],
    deps = [
        "//app/modules/v2x_application/common:app_common",
        "//app/modules/v2x_application/proto:roi_lane_cc_pb",
        "//app/modules/v2x_application/usecase/algorithm:lane_congestion",
        "//app/modules/v2x_application/usecase/algorithm:params",
        "//app/modules/v2x_application/usecase/base:config_manager",
        "//app/modules/v2x_application/usecase/base:runner",
        "//app/modules/v2x_application/usecase/common:factory",
        "//app/modules/v2x_application/usecase/common:math",
        "//app/modules/v2x_application/usecase/common:timer",
        "//app/modules/v2x_application/usecase/common:util",
        "//app/modules/v2x_application/usecase/map:map_info",
        "//app/modules/v2x_application/visualization:visualization_internal",
        "//base",
        "@local//:gflags",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
    alwayslink = True,
)
