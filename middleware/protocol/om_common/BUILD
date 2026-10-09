load("@rules_cc//cc:defs.bzl", "cc_library")

cc_binary(
    name = "libom_common.so",
    srcs = glob(
        [
            "**/*.cc",
            "**/*.cpp",
            "**/*.h*",
        ]
    ),
    linkshared = True,
    linkstatic = True,
    deps = [
        "//base",
        "@cuda",
        "@cyber",
        "@local//:boost",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:yaml-cpp",
        "@local//:protobuf",
        "@local//:paho-mqtt3as",
    ],
)

cc_library(
    name = "om_common",
    hdrs = glob(
        ["**/*.h*"],
    ),
    srcs = ["libom_common.so"],
    linkopts = [
        "-pthread  -lsqlite3",
    ],
    visibility = ["//visibility:public"],
    deps = [
            "//base",
             "@local//:fastrtps",
             "@local//:gflags",
             "@local//:glog",
             "@local//:yaml-cpp",
             "@local//:protobuf",
             "@asn_wrapper",
             "@local//:paho-mqtt3as",
        ]
)
