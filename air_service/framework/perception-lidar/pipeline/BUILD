load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "liblidar_detection_component.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    copts = [
        "-DMODULE_NAME=\\\"perception-lidar\\\"",
    ],
    linkshared = True,
    linkstatic = False,
    deps = [
        "//air_service/framework/perception-lidar/interface:base_lidar_interface",
        "//air_service/framework/perception-lidar/proto:perception_lidar_param_cc_pb",
        "//air_service/framework/proto:airos_service_pb",
        "//base",
        "//middleware/device_service/framework/lidar:lidar_service",
        "//middleware/runtime:airosrt",
        "@local//:atlas",
        "@local//:boost",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:jsoncpp",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
)
