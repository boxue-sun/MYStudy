load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libmid_camera_service.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkshared = True,
    linkstatic = True,
    copts = [
        "-DMODULE_NAME=\\\"camera_service\\\"",
    ],
    deps = [
        "//base",
        "@eigen",
    ],
)

cc_library(
    name = "mid_camera_service",
    srcs = ["libmid_camera_service.so"],
    hdrs = glob([
        "*.h",
    ]),
    visibility = ["//visibility:public"],
    deps = [
        "//base",
        "@eigen",
    ],
)
