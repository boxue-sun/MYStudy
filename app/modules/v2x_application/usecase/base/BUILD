load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "algorithm",
    srcs = [
        "base_alg.cc",
    ],
    hdrs = [
        "async_alg.h",
        "base_alg.h",
    ],
    deps = [
        ":config_manager",
        ":frame_data",
        "//app/modules/v2x_application/usecase/common",
        "//app/modules/v2x_application/usecase/common:factory",
        "//app/modules/v2x_application/usecase/common:semaphore",
        "//app/modules/v2x_application/usecase/common:timer",
    ],
)

cc_library(
    name = "config_manager",
    srcs = [
        "config_manager.cc",
    ],
    hdrs = [
        "config_manager.h",
    ],
    deps = [
        ":meta_data",
        "//app/modules/v2x_application/usecase/common",
        "//base",
        "@local//:yaml-cpp",
    ],
)

cc_library(
    name = "event_collector",
    srcs = [
        "event_collector.cc",
    ],
    hdrs = [
        "event_collector.h",
    ],
    deps = [
        ":config_manager",
        "//air_service/framework/proto:airos_service_pb",
        "//app/modules/v2x_application/usecase/common:factory",
        "//app/modules/v2x_application/usecase/common:tensor",
        "//base",
    ],
)

cc_library(
    name = "frame_data",
    srcs = [
        "frame_data.cc",
    ],
    hdrs = [
        "frame_data.h",
    ],
    deps = [
        ":event_collector",
        ":tracked_obj",
        "//app/modules/v2x_application/proto:roi_lane_cc_pb",
        "//app/modules/v2x_application/usecase/common",
        "//app/modules/v2x_application/usecase/common:timer",
        "//app/modules/v2x_application/usecase/common:util",
        "//base",
        "@local//:opencv",
    ],
)

cc_library(
    name = "meta_data",
    hdrs = [
        "meta_data.h",
    ],
    deps = [
        "//app/modules/v2x_application/usecase/common:tensor",
        "//base",
    ],
)

cc_library(
    name = "runner",
    srcs = [
        "runner.cc",
    ],
    hdrs = [
        "runner.h",
    ],
    deps = [
        ":algorithm",
        ":frame_data",
        "//app/modules/v2x_application/usecase/common",
        "//app/modules/v2x_application/usecase/common:semaphore",
        "//app/modules/v2x_application/usecase/common:timer",
    ],
)

cc_library(
    name = "tracked_obj",
    srcs = [
        "tracked_obj.cc",
    ],
    hdrs = [
        "tracked_obj.h",
    ],
    deps = [
        ":meta_data",
        "//air_service/framework/proto:airos_service_pb",
        "//app/modules/v2x_application/usecase/common:tensor",
    ],
)
