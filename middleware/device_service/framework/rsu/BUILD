load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "librsu_component.so",
    srcs = [
        "rsu_component.cc",
        "rsu_component.h",
    ],
    copts = [
        "-DMODULE_NAME=\\\"rsu_service\\\"",
    ],
    linkshared = True,
    linkstatic = True,
    deps = [
        "//base",
        "//middleware/device_service/framework/proto:rsu_config_cc_pb",
        "//middleware/runtime:airosrt",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
        "@local//:yaml-cpp",
        "//middleware/device_service/modules/rsu:librsu_device",
    ],
)
