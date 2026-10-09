load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libmulti_sensor_fusion_component.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    copts = [
        "-DMODULE_NAME=\\\"perception-fusion\\\"",
    ],
    linkshared = True,
    linkstatic = True,
    deps = [
        "//air_service/framework/perception-fusion/interface:base_fusion_interface",
        "//air_service/framework/perception-fusion/proto:fusion_config_cc_pb",
        "//air_service/framework/proto:airos_service_pb",
        "//base",
        "//middleware/runtime:airosrt",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:jsoncpp",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
)

cc_binary(
    name = "libmulti_sensor_fusion_component_viz.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    defines = ["MSF_VIZ"],
    linkshared = True,
    linkstatic = True,
    deps = [
        "//air_service/framework/perception-fusion/interface:base_fusion_interface",
        "//air_service/framework/perception-fusion/proto:fusion_config_cc_pb",
        "//air_service/modules/perception-fusion/algorithm/air_fusion:air_object_fusion",
        "//air_service/modules/perception-fusion/algorithm/interface:multi_sensor_fusion",
        "//air_service/modules/perception-fusion/proto:msf_config_cc_pb",
        "//air_service/modules/perception-fusion/visualization:msf_visualization",
        "//base",
        "//middleware/runtime:airosrt",
        "@local//:fastrtps",
        "@local//:gflags",
        "@local//:glog",
        "@local//:jsoncpp",
        "@local//:opencv",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
)
