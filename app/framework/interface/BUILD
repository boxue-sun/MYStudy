load("@rules_cc//cc:defs.bzl", "cc_library")

cc_binary(
    name = "libapp_interface.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkshared = True,
    linkstatic = True,
    deps = [
        "//app/framework/proto:airos_app_pb",
        "@local//:boost",
    ],
)

cc_library(
    name = "app_interface",
    srcs = ["libapp_interface.so"],
    hdrs = glob(["*.h"]),
    visibility = ["//visibility:public"],
    deps = [
        "//app/framework/proto:airos_app_pb",
        "@local//:boost",
    ],
)
