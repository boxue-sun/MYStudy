load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

cc_library(
    name = "libmec_device",
    srcs = ["libmec_device.so"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "libmec_device.so",
    srcs = glob(
        [
            "cictci/*.cc",
            "cictci/*.h",
        ]
    ),
    linkopts = [
        "-pthread -lz -lrt -lGeographic"
    ],
    copts = [
        "-DMODULE_NAME=\\\"mec_service\\\"",
    ],
    deps = [
        "@local//:glog",
        "@local//:yaml-cpp",
        "//base",
         "//middleware/protocol/proto:monitor_pb",
    ],
    linkshared = True,
    linkstatic = False,
)
