load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "libv2x_codec.so",
    linkshared = True,
    linkstatic = False,
    deps = [":v2x_codec_lib"],
)

cc_library(
    name = "v2x_codec_lib",
    srcs = [
        "v2x_codec_component.cc",
    ],
    hdrs = [
        "v2x_codec_component.h",
    ],
    copts = [
        "-DMODULE_NAME=\\\"v2x_codec\\\"",
    ],
    deps = [
        "//app/framework/proto:airos_app_pb",
        "//base",
        "//middleware/runtime:airosrt",
        "@asn_wrapper",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
    ],
)
