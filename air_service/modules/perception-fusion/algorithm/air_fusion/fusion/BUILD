load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "msf_fusion",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//air_service/modules/perception-fusion/algorithm/air_fusion/base:msf_base",
        "//air_service/modules/perception-fusion/proto:msf_config_cc_pb",
        "//base",
        "@eigen",
        "@local//:boost",
    ],
)
