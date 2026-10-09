load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "librsap.so",
    linkshared = True,
    linkstatic = False,
    deps = [":rsap_lib"],
)

cc_library(
    name = "rsap_lib",
    srcs = glob(
        [
            "*.cc",
            "data_model/*.cc"
        ]
    ),
    hdrs = glob(
        [
            "*.h",
            "**/*.h*",
            "data_model/*.h"
        ]
    ),
    linkopts = [
        "-pthread -lz -lrt -luuid -lsqlite3",
        "-lboost_filesystem", "-lboost_program_options", "-lboost_regex", "-lboost_system", "-lboost_thread"
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
         "@local//:boost",
        "//middleware/protocol/om_common:om_common",
        "//middleware/protocol/proto:monitor_pb",
        "//middleware/protocol/proto:monitor_mec_pb",
    ],
    copts = [
        "-DMODULE_NAME=\\\"rsap\\\""
    ],
)
