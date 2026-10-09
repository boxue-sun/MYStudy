load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libapp_demo.so",
    srcs = ["demo.cc"],
    linkshared = True,
    linkstatic = True,
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//app/framework/interface:app_interface",
        "@local//:glog",
        "@local//:protobuf",
    ],
)
