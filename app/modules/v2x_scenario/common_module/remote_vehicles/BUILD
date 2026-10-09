load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "remote_vehicle",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    deps = [
        "@local//:glog",
        "//app/framework/proto:airos_app_pb",
        "//app/modules/v2x_scenario/common_module/data_model:libdata_model",
        "//app/modules/v2x_scenario/common_module/local_maps:local_map",
        "//base"
    ],
)
