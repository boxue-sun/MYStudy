load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "parkinglot_guidance",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//app/framework/interface:app_interface",
        "@local//:glog",
        "@local//:protobuf",
        "//base",
        "//app/modules/v2x_scenario/common_module/remote_vehicles:remote_vehicle",
        "//app/modules/v2x_scenario/common_module/data_model:libdata_model"
    ],
)
