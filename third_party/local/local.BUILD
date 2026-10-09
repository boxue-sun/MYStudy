load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "yaml-cpp",
    includes = ["include"],
    linkopts = ["-lyaml-cpp"],
    linkstatic = 0,
)
cc_library(
    name = "tensorrt",
    includes = ["include"],
    linkopts = ["-lnvinfer", "-lnvinfer_plugin", "-lnvcaffe_parser", "-lnvparsers", "-lnvonnxparser"],
    linkstatic = 0,
)
cc_library(
    name = "pugixml",
    includes = ["include"],
    linkopts = ["-lpugixml"],
    linkstatic = 0,
)
cc_library(
    name = "paho-mqtt3as",
    includes = ["include"],
    linkopts = ["-lpaho-mqtt3as"],
    linkstatic = 0,
)
cc_library(
    name = "paho-mqtt3cs",
    includes = ["include"],
    linkopts = ["-lpaho-mqtt3cs"],
    linkstatic = 0,
)
cc_library(
    name = "protobuf",
    includes = ["include"],
    linkopts = ["-L/usr/local/lib", "-lprotobuf", "-lprotobuf-lite"],
    linkstatic = 0,
)
cc_library(
    name = "jsoncpp",
    includes = ["include"],
    linkopts = ["-ljsoncpp"],
    linkstatic = 0,
)
cc_library(
    name = "glog",
    includes = ["include"],
    linkopts = ["-lglog"],
    linkstatic = 0,
)
cc_library(
    name = "glfw",
    includes = ["include"],
    linkopts = ["-lglfw"],
    linkstatic = 0,
)
cc_library(
    name = "glew",
    includes = ["include"],
    linkopts = ["-lGLEW"],
    linkstatic = 0,
)
cc_library(
    name = "gflags",
    includes = ["include"],
    linkopts = ["-lgflags"],
    linkstatic = 0,
)
cc_library(
    name = "ffmpeg",
    includes = ["include"],
    linkopts = ["-lavcodec", "-lavdevice", "-lavfilter", "-lavformat", "-lavutil", "-lswscale"],
    linkstatic = 0,
)
cc_library(
    name = "fastrtps",
    includes = ["include"],
    linkopts = ["-lfastrtps", "-lfastcdr"],
    linkstatic = 0,
)
cc_library(
    name = "atlas",
    includes = ["include"],
    linkopts = ["-latlas", "-lblas", "-lcblas", "-llapack"],
    linkstatic = 0,
)
cc_library(
    name = "boost",
    includes = ["include"],
    linkopts = ["-lboost_filesystem", "-lboost_program_options", "-lboost_regex", "-lboost_system", "-lboost_thread"],
    linkstatic = 0,
)
cc_library(
    name = "opencv",
    includes = ["include"],
    linkopts = [
        "-lopencv_core",
        "-lopencv_highgui",
        "-lopencv_imgcodecs",
        "-lopencv_imgproc",
    ],
    linkstatic = 0,
)
cc_library(
    name = "opengl",
    includes = ["include"],
    linkopts = [
        "-lGL",
        "-lGLdispatch",
        "-lGLU",
        "-lGLX",
        "-lX11",
        "-lXau",
        "-lxcb",
        "-lXcursor",
        "-lXdmcp",
        "-lXinerama",
        "-lXrandr",
        "-lXxf86vm",
    ],
    linkstatic = 0,
)
