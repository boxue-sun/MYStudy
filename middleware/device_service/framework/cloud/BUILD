load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "libcloud_component.so",
    srcs = [
        "cloud_component.cc",
        "cloud_component.h",
    ],
    copts = [
        "-DMODULE_NAME=\\\"cloud_service\\\"",
    ],
    linkshared = True,
    linkstatic = True,
    deps = [
        "//base",
        "//middleware/device_service/framework/proto:cloud_config_cc_pb",
        "//middleware/runtime:airosrt",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
)
