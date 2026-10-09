load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "msf_base",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    deps = [
        "//base",
        "@abseil_cpp//absl/strings",
        "@eigen",
        "@local//:boost",
    ],
)
