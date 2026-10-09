load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "libspill_reporter.so",
    linkshared = True,
    linkstatic = False,
    deps = [":spill_reporter_lib"],
)

cc_library(
    name = "spill_reporter_lib",
    srcs = glob(
        [
            "*.cc",
        ]
    ),
    hdrs = glob(
        [
            "*.h",
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
         "@asn_wrapper",
         "@local//:paho-mqtt3as",
         "//middleware/protocol/om_common:om_common",
         "//middleware/protocol/proto:credit_control_traffic_info_pb",
         "//middleware/protocol/proto:monitor_pb",
    ],
    copts = [
        "-DMODULE_NAME=\\\"spill_reporter\\\""
    ],
)
