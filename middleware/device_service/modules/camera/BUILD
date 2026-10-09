load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cuda//cuda:defs.bzl", "cuda_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")

package(default_visibility = ["//visibility:public"])

cc_so_proto_library(
    name = "ipcamera_cc_pb",
    srcs = [":ipcamera_pb"],
)

proto_library(
    name = "ipcamera_pb",
    srcs = [
        "ipcamera/proto/header.proto",
        "ipcamera/proto/sensor_image.proto",
    ],
)

cuda_library(
    name = "apigpu-yuv2rgb",
    srcs = [
        "ipcamera/src/apigpu-yuv2rgb.cu",
    ],
    hdrs = [
        "ipcamera/include/api-structs.h",
        "ipcamera/include/apigpu-yuv2rgb.h",
    ],
    deps = [
        "//base",
        "@cuda",
        "@local//:ffmpeg",
        "@local//:glog",
    ],
)

cc_binary(
    name = "libcamera_device.so",
    srcs = glob(
        [
            "**/*.cc",
            "**/*.h",
            "**/**/*.cc",
            "**/**/*.h",
        ],
        exclude = [
            "ut/camera_test.cc",
            "test_tool/test_tool.cc",
            "ipcamera/apps/apidemos.cc",
            "ipcamera/src/dahua_camera.cc",
        ],
    ),
    includes = [
        ".",
    ],
    linkshared = True,
    linkstatic = True,
    copts = [
        "-DMODULE_NAME=\\\"camera_service\\\"",
    ],
    deps = [
        "apigpu-yuv2rgb",
        "ipcamera_cc_pb",
        "//base",
        "//middleware/runtime:airosrt",
        "@cuda",
        "@hiksdk",
        "@local//:fastrtps",
        "@local//:ffmpeg",
        "@local//:gflags",
        "@local//:glog",
        "@local//:opencv",
        "@local//:protobuf",
    ],
)
