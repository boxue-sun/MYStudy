load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "hiksdk",
    srcs = [
        "lib/libanalyzedata.so",
        "lib/libAudioRender.so",
        "lib/libHCCore.so",
        "lib/libhcnetsdk.so",
        "lib/libhpr.so",
        "lib/libPlayCtrl.so",
        "lib/libSuperRender.so",
    ],
    hdrs = glob(["include/*.h"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
    linkstatic = 0,
)
