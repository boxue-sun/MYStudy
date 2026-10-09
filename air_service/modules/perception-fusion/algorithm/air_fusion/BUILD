load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "air_object_fusion",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    deps = [
        "//air_service/modules/perception-fusion/algorithm/air_fusion/fusion:msf_fusion",
        "//air_service/modules/perception-fusion/algorithm/interface:multi_sensor_fusion",
        "//base",
        "@eigen",
        "@local//:boost",
        "@local//:opencv",
    ],
)
