load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cooperation_service_announcement",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    deps = [
        "//app/framework/interface:app_interface",
        "@local//:glog",
        "@local//:protobuf",
        "//base",
    ],
)
