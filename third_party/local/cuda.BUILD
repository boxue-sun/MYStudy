load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cuda",
    includes = ["include"],
    linkopts = [
        "-L/usr/local/cuda/lib64/stubs",
        "-L/usr/local/cuda/lib64",
        "-lcublas",
        "-lcublasLt",
        "-lcudart",
        "-lcudnn",
        "-lcurand",
        "-lnppc",
        "-lnppicc",
        "-lnppidei",
        "-lnppig",
        "-lcuda",
    ],
    hdrs = glob([
        "include/**",
    ]),
    linkstatic = 0,
)
