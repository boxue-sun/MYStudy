load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "radar_device",
    srcs = glob(
        [
            "**/*.cc",
            "**/*.h",
        ],
        exclude = ["ut/radar_test.cc"],
    ),
    deps = [
        "//base",
    ],
)

cc_test(
    name = "radar_ut",
    srcs = [
        "ut/radar_test.cc",
    ],
    includes = [
        ".",
    ],
    deps = [
        ":radar_device",
        "//base",
        "@com_google_googletest//:gtest_main",
        "@local//:protobuf",
    ],
)
