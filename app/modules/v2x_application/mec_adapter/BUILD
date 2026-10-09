load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libmec_adapter.so",
    linkshared = True,
    linkstatic = False,
    deps = [":mec_adapter_lib"],
)

cc_library(
    name = "mec_adapter_lib",
    srcs = ["mec_adapter.cc"],
    hdrs = ["mec_adapter.h"],
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//app/framework/interface:app_interface",
        "//app/framework/proto:airos_app_pb",
        "//app/modules/v2x_application/common:app_common",
        "//base",
        "@local//:gflags",
        "@local//:glog",
    ],
)
