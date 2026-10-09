load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

cc_so_proto_library(
    name = "airos_plugin_pb",
    srcs = [":dynamic_modules_cfg_pb"],
)

proto_library(
    name = "dynamic_modules_cfg_pb",
    srcs = [
        "plugin/algorithm_loader/dynamic_modules_cfg.proto",
        "plugin/modules_loader/app_lib_cfg.proto",
        "plugin/modules_loader/device_lib_cfg.proto",
    ],
)

cc_binary(
    name = "libairos_base.so",
    srcs = glob(
        [
            "**/*.cc",
            "**/*.cpp",
            "**/*.h*",
        ],
        exclude = ["**/test/*"],
    ),
    linkshared = True,
    linkstatic = True,
    linkopts = [
        "-lcurl",
    ],
    deps = [
        ":airos_plugin_pb",
        "//base/device_connect/proto:airos_device_pb",
        "//air_service/framework/proto:airos_service_pb",
        "@cuda",
        "@cyber",
        "@local//:boost",
        "@local//:glog",
        "@local//:yaml-cpp",
        "@local//:paho-mqtt3cs",
    ],
)

cc_library(
    name = "base",
    srcs = ["libairos_base.so"],
    hdrs = glob(
        ["**/*.h*"],
        exclude = ["**/test/*"],
    ),
    visibility = ["//visibility:public"],
    deps = [
        "//base/device_connect/proto:airos_device_pb",
        "//air_service/framework/proto:airos_service_pb",
        "@cuda",
        "@cyber",
    ],
)
