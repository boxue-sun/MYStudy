load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "libom.so",
    linkshared = True,
    linkstatic = False,
    deps = [":om_lib"],
)

cc_library(
    name = "om_lib",
    srcs = glob(
        [
            "*.cc",
        ]
    ),
    hdrs = glob(
        [
            "*.h",
            "configer/*.h",
            "data_model/mec/*.h",
            "data_model/camera/*.h",
            "data_model/radar/*.h",
        ]
    ),
    linkopts = [
        "-pthread -lz -lrt -luuid -lcurl -lsqlite3",
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
         "@asn_wrapper",
         "@local//:paho-mqtt3as",
         "@local//:pugixml",
         "//middleware/protocol/om_common:om_common",
         "//base/device_connect/proto:airos_device_pb",
          "//middleware/protocol/proto:monitor_pb",
         "//middleware/protocol/proto:monitor_mec_pb",
    ],
    copts = [
        "-DMODULE_NAME=\\\"om\\\""
    ],
)
