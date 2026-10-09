load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libcloud_adapter.so",
    linkshared = True,
    linkstatic = False,
    deps = [":cloud_adapter_lib"],
)

cc_library(
    name = "cloud_adapter_lib",
    srcs = ["cloud_adapter.cc"],
    hdrs = ["cloud_adapter.h"],
    deps = [
        "//app/framework/proto:airos_app_pb",
        "//app/modules/v2x_application/common:app_common",
        "//base",
        "@local//:gflags",
        "@local//:glog",
    ],
)
