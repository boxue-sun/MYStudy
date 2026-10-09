load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libcamera_visualiztion_component.so",
    linkshared = True,
    linkstatic = False,
    deps = [":camera_visualiztion_component_lib"],
)

cc_library(
    name = "camera_visualiztion_component_lib",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    deps = [
        "//air_service/framework/perception-camera/proto:perception_camera_param_cc_pb",
        "//air_service/framework/proto:airos_service_pb",
        "//air_service/modules/perception-visualization/config:perception_visualization_config",
        "//air_service/modules/perception-visualization/visualization/base:perception_visualization_base",
        "//air_service/modules/perception-visualization/visualization/visualizer:perception_visualization_visualizer",
        "//base",
        "//middleware/device_service/framework/camera:mid_camera_service",
        "//middleware/runtime:airosrt",
        "@eigen",
        "@local//:gflags",
        "@local//:glew",
        "@local//:glfw",
        "@local//:glog",
        "@local//:jsoncpp",
        "@local//:protobuf",
        "@local//:yaml-cpp",
    ],
)

cc_library(
    name = "camera_visualiztion_message",
    hdrs = glob([
        "camera_perception_viz_message.h",
    ]),
    deps = [
        "//air_service/modules/perception-visualization/visualization/base:perception_visualization_base",
        "//base",
        "@eigen",
    ],
)
