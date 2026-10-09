load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "local_map",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkopts = [
        "-lGeographic"
    ],
    deps = [
        "@local//:glog",
        "//app/framework/proto:airos_app_pb",
        "//app/modules/v2x_scenario/common:app_common",
        "@asn_wrapper",
        "//base",
        "//app/modules/v2x_application/usecase/common:math"
    ],
)
