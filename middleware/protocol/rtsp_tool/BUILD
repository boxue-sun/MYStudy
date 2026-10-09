load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "rtsp_tool",
    srcs = [
        "rtsp_tool.cc",
        "rtsp_pusher.cc",
        "rtsp_pusher.h",
        "channel_data_processor.cc",
        "channel_data_processor.h",
    ],
    copts = [
        "-std=c++14",
    ],
    deps = [
        "//base",
        "@local//:gflags",
        "@local//:glog",
        "@local//:protobuf",
        "@local//:ffmpeg",
    ],
    linkopts = [
        "-lpthread",
    ],
)
