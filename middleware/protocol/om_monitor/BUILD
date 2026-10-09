load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "libom_monitor.so",
    linkshared = True,
    linkstatic = False,
    deps = [":om_monitor_lib"],
)

cc_library(
    name = "om_monitor_lib",
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
          "//middleware/protocol/proto:monitor_pb",
    ],
    copts = [
        "-DMODULE_NAME=\\\"om_monitor\\\""
    ],
)
