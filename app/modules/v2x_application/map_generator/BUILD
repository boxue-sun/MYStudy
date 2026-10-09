load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libmap_generator.so",
    linkshared = True,
    linkstatic = False,
    deps = [":map_generator_lib"],
)

cc_library(
    name = "map_generator_lib",
    srcs = ["map_generator.cc"],
    hdrs = ["map_generator.h"],
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//app/framework/interface:app_interface",
        "//app/framework/proto:airos_app_pb",
        "//app/modules/v2x_application/common:app_common",
        "//base",
        "@local//:gflags",
        "@local//:glog",
        "@asn_wrapper",
    ],
)
