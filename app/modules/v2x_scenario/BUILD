load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libv2x_scenario.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkshared = True,
    linkstatic = True,
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//app/framework/interface:app_interface",
        "@local//:glog",
        "@local//:protobuf",
        "//base",
        "//app/modules/v2x_scenario/common_module/remote_vehicles:remote_vehicle",
        "//app/modules/v2x_scenario/cooperation_intersection_cross:cooperation_intersection_cross",
        "//app/modules/v2x_scenario/common_module/traffic_light_data:traffic_light_data",
        "//app/modules/v2x_scenario/cooperation_rampin:cooperation_rampin",
        "//app/modules/v2x_scenario/cooperation_lanechange:cooperation_lanechange",
        "//app/modules/v2x_scenario/cooperation_service_announcement:cooperation_service_announcement",
        "//app/modules/v2x_scenario/parkinglot_guidance:parkinglot_guidance"
    ],
)
