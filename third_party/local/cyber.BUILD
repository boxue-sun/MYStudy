load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_python//python:defs.bzl", "py_library")

cc_library(
    name = "cyber",
    includes = ["include"],
    hdrs = glob(["include/**/*"]),
    srcs = glob(["lib/**/lib*.so*"]),
    include_prefix = "cyber",
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)

# 暴露镜像内 /opt/local/cyber-rt/python/cyber/python/cyber_py3 下的python模块
# 供点云录制等python组件通过 @cyber//:cyber_py3 依赖
py_library(
    name = "cyber_py3",
    # 对应物理路径: /opt/local/cyber-rt/python/cyber/python/cyber_py3
    srcs = glob(["python/cyber/python/cyber_py3/**/*.py"]),
    # 将该目录加入 PYTHONPATH，使代码里可以直接 import
    imports = ["python/cyber/python/cyber_py3"],
    visibility = ["//visibility:public"],
)
