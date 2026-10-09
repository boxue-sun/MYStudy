load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "libom_device_status.so",
    linkshared = True,
    linkstatic = False,
    deps = [":om_device_status_lib"],
)

cc_library(
    name = "om_device_status_lib",
    srcs = glob(
        [
            "*.cc",
        ]
    ),
    hdrs = glob(
        [
            "*.h",
            "data_model/*.h",
        ]
    ),
    linkopts = [
        "-pthread -lz -lrt -luuid",
    ],
    deps = [
        "//base",
        "//app/framework/proto:airos_app_pb",
        "//middleware/runtime:airosrt",
         "//air_service/framework/proto:airos_service_pb",
         "@local//:fastrtps",
         "@local//:gflags",
         "@local//:glog",
         "@local//:yaml-cpp",
         "@local//:protobuf",
         "@local//:paho-mqtt3as",
         "//middleware/protocol/om_common:om_common",
    ],
    copts = [
        "-DMODULE_NAME=\\\"om_device_status\\\""
    ],
)
