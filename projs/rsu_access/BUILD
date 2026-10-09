load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "librsu_access.so",
    srcs = glob([
        "standard_rsu/protocol/*",
        "standard_rsu/*",
    ]),
    linkshared = True,
    linkstatic = True,
    deps = [
        "@airos_sdk//airos",
        "@local//:glog",
        "@local//:yaml-cpp",
    ],
)
