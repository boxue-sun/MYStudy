load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libairos_perception_lidar.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    copts = [
        "-DMODULE_NAME=\\\"perception-lidar\\\"",
    ],
    linkshared = True,
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        "//air_service/framework/perception-lidar/interface:base_lidar_interface",
        "//air_service/framework/proto:airos_service_pb",
        "//base",
        "@local//:glog",
        "@local//:protobuf",
    ],
)
