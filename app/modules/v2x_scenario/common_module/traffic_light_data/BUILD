load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "traffic_light_data",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    deps = [
        "@local//:glog",
        "//app/framework/proto:airos_app_pb",
        "//base"
    ],
)
