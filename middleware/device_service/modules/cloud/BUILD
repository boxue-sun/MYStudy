load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "cloud_device",
    srcs = glob(
        [
            "standard_mqtt/*.cc",
        ],
    ),
    hdrs = glob(
        [
            "standard_mqtt/*.h",
        ],
    ),
    linkstatic = False,
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//base",
        "@local//:glog",
        "@local//:paho-mqtt3cs",
        "@local//:yaml-cpp",
    ],
)

cc_test(
    name = "cloud_ut",
    srcs = ["ut/cloud_test.cc"],
    deps = [
        ":cloud_device",
        "@com_google_googletest//:gtest_main",
    ],
)
