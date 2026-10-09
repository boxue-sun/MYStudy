load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "libdata_model",
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
        "//base",
        "//app/modules/v2x_scenario/common_module/local_maps:local_map",
        "//app/modules/v2x_application/usecase/common:math"
    ],
)
