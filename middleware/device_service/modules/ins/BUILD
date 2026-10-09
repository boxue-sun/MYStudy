load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "ins_device",
    srcs = glob(
        [
            "**/*.cc",
            "**/*.h",
        ],
        exclude = [
            "ut/ins_test.cc",
            "test_tool/test_tool.cc",
        ],
    ),
    deps = [
        "//base",
    ],
)

cc_test(
    name = "ins_ut",
    srcs = [
        "ut/ins_test.cc",
    ],
    includes = [
        ".",
    ],
    deps = [
        ":ins_device",
        "//base",
        "@com_google_googletest//:gtest_main",
        "@local//:protobuf",
    ],
)

cc_binary(
    name = "ins_test_tool",
    srcs = [
        "test_tool/test_tool.cc",
    ],
    includes = [
        ".",
    ],
    deps = [
        ":ins_device",
        "//base",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
    linkopts = ["-lpthread"],
)
