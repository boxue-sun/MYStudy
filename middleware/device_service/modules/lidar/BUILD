load("@rules_cc//cc:defs.bzl", "cc_library")

cc_binary(
    name = "liblidar_device.so",
    srcs = glob([
            "**/*.cc",
            "**/*.h",
            ],
            exclude = [
                "ut/lidar_test.cc",
            ],
    ),
    linkshared = True,
    linkstatic = True,
    copts = [
        "-DMODULE_NAME=\\\"lidar_service\\\"",
    ],
    deps = [
        "//base",
        "@local//:glog",
    ],
)

cc_test(
    name = "lidar_ut",
    srcs = [
        "ut/lidar_test.cc",
        "example/dummy_lidar.cc",
        "example/dummy_lidar.h",
    ],
    deps = [
        "//base",
        "@com_google_googletest//:gtest_main",
        "@local//:protobuf",
    ],
)
