load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "asn_wrapper",
    srcs = glob(["lib/*.so"]),
    hdrs = glob(["include/*"]),
    strip_include_prefix = "include",
    include_prefix = "v2xpb-asn",
    visibility = ["//visibility:public"],
    linkstatic = 0,
)
