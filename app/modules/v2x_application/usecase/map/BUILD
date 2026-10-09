load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "map_info",
    srcs = [
        "map_info.cc",
    ],
    hdrs = [
        "map_info.h",
    ],
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//app/modules/v2x_application/proto:roi_lane_cc_pb",
        "//app/modules/v2x_application/usecase/algorithm:params",
        "//app/modules/v2x_application/usecase/base:config_manager",
        "//app/modules/v2x_application/usecase/common:factory",
        "//app/modules/v2x_application/usecase/common:math",
        "//app/modules/v2x_application/usecase/common:util",
        "//base",
        "@local//:gflags",
        "@local//:pugixml",
        "@local//:yaml-cpp",
    ],
)
