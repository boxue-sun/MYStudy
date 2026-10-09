load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "libsound_player.so",
    linkshared = True,
    linkstatic = False,
    deps = [":sound_player_lib"],
)

cc_library(
    name = "sound_player_lib",
    srcs = glob(
        [
            "*.cc",
        ]
    ),
    hdrs = glob(
        [
            "*.h",
        ]
    ),
    linkopts = [
        "-pthread -lz -lrt -luuid",
    ],
    deps = [
        "//base",
        "//app/framework/proto:airos_app_pb",
        "//middleware/runtime:airosrt",
         "//air_service/framework/proto:airos_service_pb",
         "@local//:fastrtps",
         "@local//:gflags",
         "@local//:glog",
         "@local//:yaml-cpp",
         "@local//:protobuf",
         "//middleware/protocol/om_common:om_common",
    ],
    copts = [
        "-DMODULE_NAME=\\\"sound_player\\\""
    ],
)