load("@rules_python//python:defs.bzl", "py_library")
package(default_visibility = ["//visibility:public"])

pb_deps = ["@com_google_protobuf//:protobuf_python"]

py_library(
    name = "dian_yun_recorder",
    srcs = ["DianYunRecorder.py"],
    deps = [
        # 修改这里：指向 @cyber 根目录下的 cyber_py3 目标
        "@cyber//:cyber_py3",

        "//middleware/protocol/proto:monitor_mec_py_pb2",
    ] + pb_deps,
)
