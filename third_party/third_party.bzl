load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def clean_dep(dep):
    return str(Label(dep))

def init_third_party():
    native.new_local_repository(
        name = "local",
        build_file = clean_dep("//third_party/local:local.BUILD"),
        path = "/usr/local",
    )
    native.new_local_repository(
        name = "cuda",
        build_file = clean_dep("//third_party/local:cuda.BUILD"),
        path = "/usr/local/cuda",
    )
    http_archive(
        name = "com_google_googletest",
        sha256 = "9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb",
        strip_prefix = "googletest-release-1.10.0",
        urls = [
            "https://zhilu.bj.bcebos.com/googletest-release-1.10.0.tar.gz",
        ],
    )
    native.new_local_repository(
        name = "cyber",
        path = "/opt/local/cyber-rt",
        build_file = clean_dep("//third_party/local:cyber.BUILD"),
    )
    native.new_local_repository(
        name = "hiksdk",
        build_file = clean_dep("//third_party/local:hiksdk.BUILD"),
        path = "/opt/local/Hikvision",
    )
    native.new_local_repository(
        name = "paddle",
        build_file = clean_dep("//third_party/local:paddle.BUILD"),
        path = "/opt/local/paddle",
    )
    native.new_local_repository(
        name = "asn_wrapper",
        build_file = clean_dep("//third_party/local:asn_wrapper.BUILD"),
        path = "/opt/local/asn-wrapper",
    )
    native.new_local_repository(
        name = "eigen",
        build_file = clean_dep("//third_party/local:eigen.BUILD"),
        path = "/usr",
    )
