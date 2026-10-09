load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "polygon_collect",
    srcs = [
        "polygon_collect.cc",
    ],
    hdrs = [
        "polygon_collect.h",
    ],
    deps = [
        "//app/modules/v2x_application/usecase/common:math",
        "//app/modules/v2x_application/usecase/common:tensor",
        "//base",
        "@local//:opencv",
    ],
)

cc_library(
    name = "visualization_internal",
    srcs = [
        "visualization_internal.cc",
    ],
    hdrs = [
        "visualization_internal.h",
    ],
    deps = [
        ":polygon_collect",
        ":window_wrapper",
        "//air_service/framework/proto:airos_service_pb",
        "//app/modules/v2x_application/usecase/common:tensor",
        "//base",
    ],
)

cc_library(
    name = "window_wrapper",
    srcs = [
        "window_wrapper.cc",
    ],
    hdrs = [
        "window_wrapper.h",
    ],
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//app/modules/v2x_application/proto:roi_lane_cc_pb",
        "//app/modules/v2x_application/usecase/base:config_manager",
        "//app/modules/v2x_application/usecase/base:runner",
        "//app/modules/v2x_application/usecase/common:factory",
        "//app/modules/v2x_application/usecase/common:math",
        "//app/modules/v2x_application/usecase/common:util",
        "//base",
        "@eigen",
        "@local//:glog",
        "@local//:opencv",
        "@local//:yaml-cpp",
    ],
)
