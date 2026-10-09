load("@rules_proto//proto:defs.bzl", "proto_library")
load("//tools:cc_so_proto_rules.bzl", "cc_so_proto_library")
load("@com_google_protobuf//:protobuf.bzl", "py_proto_library")
pb_deps = ["@com_google_protobuf//:protobuf_python"]
package(default_visibility = ["//visibility:public"])


cc_so_proto_library(
    name = "credit_control_traffic_info_pb",
    srcs = [":traffic_info_pb"],
)

proto_library(
    name = "traffic_info_pb",
    srcs = ["st_header.proto","cross.proto","line.proto","traffic_info.proto"],
)


cc_so_proto_library(
    name = "monitor_pb",
    srcs = [":monitor_info_pb"],
)

proto_library(
    name = "monitor_info_pb",
    srcs = ["monitor.proto"],
)

cc_so_proto_library(
    name = "monitor_mec_pb",
    srcs = [":monitor_mec_info_pb"],
)

proto_library(
    name = "monitor_mec_info_pb",
    srcs = ["monitor_mec.proto"],
)

py_proto_library(
    name = "monitor_mec_py_pb2",
    srcs = ["monitor_mec.proto"], # 这里写你实际的 .proto 文件名
    deps = [
        "@com_google_protobuf//:protobuf_python",
    ],
)
