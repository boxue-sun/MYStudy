load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libcamera_detection_component.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    copts = [
        "-DMODULE_NAME=\\\"perception-camera\\\"",
    ],
    linkshared = True,
    linkstatic = False,
    deps = [
        "//air_service/framework/perception-camera/interface:base_perception_interface",
        "//air_service/framework/perception-camera/proto:perception_camera_param_cc_pb",
        "//air_service/framework/proto:airos_service_pb",
        "//base",
        "//middleware/device_service/framework/camera:mid_camera_service",
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

cc_binary(
    name = "libcamera_detection_component_viz.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    defines = ["PERCEPTION_VIZ"],
    linkshared = True,
    linkstatic = False,
    deps = [
        "//air_service/framework/perception-camera/interface:base_perception_interface",
        "//air_service/framework/proto:airos_service_pb",
        "//air_service/modules/perception-camera/algorithm/detector:detector_plugin_adapter",
        "//air_service/modules/perception-camera/algorithm/interface:camera_algorithm_interface",
        "//air_service/modules/perception-camera/algorithm/roi_filter:roi_filter_adapter",
        "//air_service/modules/perception-camera/algorithm/tracker:tracker_plugin_adapter",
        "//air_service/modules/perception-camera/algorithm/transformer:transformer_plugin_adapter",
        "//air_service/modules/perception-camera/proto:perception_param_cc_pb",
        "//air_service/modules/perception-visualization/pipline:camera_visualiztion_component_lib",
        "//base",
        "//middleware/device_service/framework/camera:mid_camera_service",
        "//middleware/runtime:airosrt",
        "@eigen",
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
