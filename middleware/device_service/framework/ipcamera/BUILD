load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "libipcamera_component.so",
    linkshared = True,
    linkstatic = False,
    deps = [":ipcamera_component_lib"],
)

cc_library(
    name = "ipcamera_component_lib",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    linkopts = [
        "-pthread",
    ],
    deps = [
        "//base",
        "//middleware/device_service/framework/proto:ipcamera_config_cc_pb",
        "//middleware/runtime:airosrt",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
        "@local//:yaml-cpp",
        "//middleware/device_service/modules/ipcamera:libipcamera_device",
    ],
    copts = [
        "-DMODULE_NAME=\\\"ipcamera_service\\\"",
    ],
) 