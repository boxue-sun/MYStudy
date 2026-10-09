load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "libmec_component.so",
    linkshared = True,
    linkstatic = False,
    deps = [":mec_component_lib"],
)

cc_library(
    name = "mec_component_lib",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    linkopts = [
        "-pthread",
    ],
    deps = [
        "//base",
        "//middleware/device_service/framework/proto:mec_config_cc_pb",
        "//middleware/runtime:airosrt",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
        "@local//:yaml-cpp",
        "//middleware/device_service/modules/mec:libmec_device",
        "//middleware/protocol/proto:monitor_pb",
        "//middleware/protocol/proto:monitor_mec_pb",
    ],
    copts = [
        "-DMODULE_NAME=\\\"mec_service\\\"",
    ],
)

