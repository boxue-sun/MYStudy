load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "airos_app_framework",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkstatic = False,
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//app/framework/interface:app_interface",
        "//app/framework/proto:airos_app_pb",
        "//base",
        "//middleware/runtime:airosrt",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
    ],
    linkopts = ["-lpthread"],
)
