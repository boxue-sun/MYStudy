load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "libipcamera_device",
    srcs = ["libipcamera_device.so"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "libipcamera_device.so",
    srcs = glob([
        "standard_ipcamera/*",
    ]),
    linkshared = True,
    linkstatic = False,
    copts = [
        "-DMODULE_NAME=\\\"ipcamera_service\\\"",
    ],
    deps = [
        "//base",
        "//base/device_connect/proto:airos_device_pb",
        "@local//:glog",
        "@local//:yaml-cpp",
        "@local//:ffmpeg",
    ],
) 