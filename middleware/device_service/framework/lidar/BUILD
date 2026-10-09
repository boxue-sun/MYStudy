load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "liblidar_service.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkshared = True,
    linkstatic = True,
    copts = [
        "-DMODULE_NAME=\\\"lidar_service\\\"",
    ],
    deps = [
        "//base",
        "@eigen",
    ],
)

cc_library(
    name = "lidar_service",
    srcs = ["liblidar_service.so"],
    hdrs = glob([
        "*.h",
    ]),
    visibility = ["//visibility:public"],
    deps = [
        "//base",
        "@eigen",
    ],
)
