load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "app_common",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//base",
        "@local//:gflags",
        "@local//:glog",
        "@local//:paho-mqtt3cs",
    ],
)
