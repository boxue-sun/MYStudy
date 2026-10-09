load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "libv2x_app.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkshared = True,
    linkstatic = True,
    deps = [
        "//air_service/framework/proto:airos_service_pb",
        "//app/framework/interface:app_interface",
        "//app/framework/proto:airos_app_pb",
        "//app/modules/v2x_application/cloud_adapter:cloud_adapter_lib",
        "//app/modules/v2x_application/common:app_common",
        "//app/modules/v2x_application/monitor:monitor_lib",
        "//app/modules/v2x_application/perception_adapter:perception_adapter_lib",
        "//app/modules/v2x_application/rsi_generator:rsi_generator_lib",
        "//app/modules/v2x_application/traffic_light_adapter:traffic_light_adapter_lib",
        "//app/modules/v2x_application/usecase_adapter",
        "//app/modules/v2x_application/v2x_message_reporter:v2x_message_reporter_lib",
        "//app/modules/v2x_application/mec_adapter:mec_adapter_lib",
        "//app/modules/v2x_application/map_generator:map_generator_lib",
        "//app/modules/v2x_application/mec_adapter_ssm:mec_adapter_ssm_lib",
        "@asn_wrapper",
        "@local//:gflags",
        "@local//:glog",
    ],
)
