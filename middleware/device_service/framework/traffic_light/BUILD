load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "libtraffic_light_component.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    copts = [
        "-DMODULE_NAME=\\\"traffic_light_service\\\"",
    ],
    linkshared = True,
    linkstatic = True,
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//base",
        "//middleware/device_service/framework/proto:traffic_light_config_cc_pb",
        "//middleware/runtime:airosrt",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:jsoncpp",
        "@local//:protobuf",
        "@local//:yaml-cpp",
        "//middleware/device_service/modules/traffic_light:libtraffic_light_device",
       "//middleware/protocol/proto:monitor_pb",
       "//middleware/protocol/proto:monitor_mec_pb",
    ],
)
