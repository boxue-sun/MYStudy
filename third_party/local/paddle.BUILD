load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "paddle",
    srcs = glob([
        "lib/libpaddle_inference.so",
        "lib/libiomp5.so",
        "lib/libmklml_intel.so",
    ]),
    hdrs = glob([
        "include/*.h",
        "include/**/*.h",
    ]),
    includes = [
        "include",
    ],
    visibility = ["//visibility:public"],
    linkstatic = 0,
)
