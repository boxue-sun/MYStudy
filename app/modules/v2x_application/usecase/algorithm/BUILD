load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "lane_congestion",
    srcs = [
        "lane_congestion.cc",
    ],
    hdrs = [
        "lane_congestion.h",
    ],
    deps = [
        "//app/modules/v2x_application/usecase/base:algorithm",
        "//app/modules/v2x_application/usecase/common:factory",
        "//app/modules/v2x_application/usecase/common:math",
        "//app/modules/v2x_application/usecase/map:map_info",
        "//app/modules/v2x_application/visualization:polygon_collect",
        "//base",
    ],
)

cc_library(
    name = "params",
    srcs = [
        "params.cc",
    ],
    hdrs = [
        "params.h",
    ],
    deps = [
        "//app/modules/v2x_application/usecase/base:config_manager",
        "//app/modules/v2x_application/usecase/common:factory",
    ],
)
