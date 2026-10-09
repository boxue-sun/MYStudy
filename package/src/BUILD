load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_cc//cc:defs.bzl", "cc_binary")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "airospkg",
    srcs = [
        "airos_pack.cc",
        "airos_pack.h",
        "create_template.cc",
        "create_template.h",
        "maco_params.cc",
        "maco_params.h",
        "main.cc",
        "show_help.cc",
        "show_help.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@local//:gflags",
        "@local//:jsoncpp",
    ],
)
