load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    hdrs = [
        "common.h",
    ],
)

cc_library(
    name = "tensor",
    srcs = [
        "tensor.cc",
    ],
    hdrs = [
        "tensor.h",
    ],
)

cc_library(
    name = "util",
    srcs = [
        "util.cc",
    ],
    hdrs = [
        "util.h",
    ],
)

cc_library(
    name = "factory",
    hdrs = [
        "factory.hpp",
    ],
)

cc_library(
    name = "semaphore",
    hdrs = [
        "semaphore.h",
    ],
)

cc_library(
    name = "timer",
    hdrs = [
        "timer.h",
    ],
    deps = [
        ":common",
    ],
)

cc_library(
    name = "math",
    srcs = [
        "polygon2d.cc",
        "segment2d.cc",
    ],
    hdrs = [
        "math_utils.h",
        "polygon2d.h",
        "segment2d.h",
        "vec2d.h",
    ],
    deps = [
        "//base",
        "@local//:glog",
    ],
)
