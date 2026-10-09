FROM docker.io/nvidia/cuda:11.4.3-devel-ubuntu20.04 AS base
# FROM docker.io/nvidia/cuda:11.6.2-devel-ubuntu20.04 AS base

SHELL ["/bin/bash", "-c"]

ENV  DEBIAN_FRONTEND noninteractive
COPY tuna-ubuntu-2004-sources.list    /etc/apt/sources.list
RUN yes | unminimize
RUN apt  update                                                            \
 && apt  install -y --no-install-recommends                                \
         zip unzip bzip2 xz-utils gzip zstd vim less file gawk telnet lsof \
         sudo wget curl udev ssh iputils-ping git git-gui gitk git-lfs zsh \
         jq locales locales-all lsb-release webp gfortran rsync man tree   \
         kmod meld default-jre openssl squashfs-tools rsyslog bc           \
                                                                           \
         build-essential autoconf automake bison flex libtool-bin          \
         ltrace strace help2man texinfo device-tree-compiler g++-7         \
         ninja-build cmake nasm                                            \
                                                                           \
         python3                       python                              \
         python3-all-dev               python-all-dev                      \
         python3-crypto                python-crypto                       \
         python3-pexpect               python-pexpect                      \
         python3-setuptools            python-setuptools                   \
         python3-six                   python-six                          \
         python3-yaml                  python-yaml                         \
         python3-requests                                                  \
         python3-pandas                                                    \
         python3-pip                                                       \
                                                                           \
         gstreamer1.0-plugins-base                                         \
         gstreamer1.0-plugins-good                                         \
         libatlas-base-dev                                                 \
         libboost-all-dev                                                  \
         libbz2-dev                                                        \
         libcurl4-openssl-dev                                              \
         libeigen3-dev                                                     \
         libffi-dev                                                        \
         libfreetype6-dev                                                  \
         libgbm1                                                           \
         libgl1-mesa-dev                                                   \
         libglew-dev                                                       \
         libglfw3-dev                                                      \
         libglm-dev                                                        \
         libgstreamer1.0-dev                                               \
         libgtk2.0-dev                                                     \
         libhdf5-dev                                                       \
         libjsoncpp-dev                                                    \
         liblapack-dev                                                     \
         libleveldb-dev                                                    \
         liblz4-dev                                                        \
         liblzma-dev                                                       \
         libncurses5                                                       \
         libncurses-dev                                                    \
         libnuma-dev                                                       \
         libopenblas-dev                                                   \
         libpugixml-dev                                                    \
         libsdl2-dev                                                       \
         libsdl2-image-dev                                                 \
         libsnappy-dev                                                     \
         libssl-dev                                                        \
         libtheora0                                                        \
         libunwind-dev                                                     \
         libv4l-dev                                                        \
         libva-dev                                                         \
         libva-drm2                                                        \
         libvdpau1                                                         \
         libvorbisenc2                                                     \
         libyaml-cpp-dev                                                   \
         libz-dev                                                          \
         uuid-dev                                                          \
         libgeographic-dev                                                 \
 && :

# --------------------------------------------------------------------------
# ENV CUDNN_VERSION="8.4.1.50-1+cuda11.6"
# ENV TERNSOR_RT_VERSION="8.4.1-1+cuda11.6"
ENV CUDNN_VERSION="8.2.2.26-1+cuda11.4"
ENV TERNSOR_RT_VERSION="8.2.2-1+cuda11.4"

# RUN apt install -y --no-install-recommends                                 \
#         "libcudnn8=${CUDNN_VERSION}"                                       \
#         "libcudnn8-dev=${CUDNN_VERSION}"                                   \
#         "libnvinfer-bin=${TERNSOR_RT_VERSION}"                             \
#         "libnvinfer-dev=${TERNSOR_RT_VERSION}"                             \
#         "libnvinfer-plugin-dev=${TERNSOR_RT_VERSION}"                      \
#         "libnvinfer-plugin8=${TERNSOR_RT_VERSION}"                         \
#         "libnvinfer8=${TERNSOR_RT_VERSION}"                                \
#         "libnvonnxparsers-dev=${TERNSOR_RT_VERSION}"                       \
#         "libnvonnxparsers8=${TERNSOR_RT_VERSION}"                          \
#         "libnvparsers-dev=${TERNSOR_RT_VERSION}"                           \
#         "libnvparsers8=${TERNSOR_RT_VERSION}"                              \
#  && :

RUN apt install -y --no-install-recommends                                 \
        "libcudnn8=${CUDNN_VERSION}"                                       \
        "libcudnn8-dev=${CUDNN_VERSION}"                                   \
        "libnvinfer-dev=${TERNSOR_RT_VERSION}"                             \
        "libnvinfer-plugin-dev=${TERNSOR_RT_VERSION}"                      \
        "libnvinfer-plugin8=${TERNSOR_RT_VERSION}"                         \
        "libnvinfer8=${TERNSOR_RT_VERSION}"                                \
        "libnvonnxparsers-dev=${TERNSOR_RT_VERSION}"                       \
        "libnvonnxparsers8=${TERNSOR_RT_VERSION}"                          \
        "libnvparsers-dev=${TERNSOR_RT_VERSION}"                           \
        "libnvparsers8=${TERNSOR_RT_VERSION}"                              \
 && :

COPY tuna-ubuntu-2004-u18-compat.list /etc/apt/sources.list.d/
RUN apt  update                                                            \
 && apt  install -y --no-install-recommends                                \
         libpython3.6/bionic-security                                      \
 && :

# ==========================================================================
FROM base AS build_usr_local
WORKDIR /tmp
# --------------------------------------------------------------------------
ARG PKG=paho.mqtt.c
ARG VER=1.3.13
RUN wget -q --content-disposition                                          \
    https://github.com/eclipse/${PKG}/archive/refs/tags/v${VER}.tar.gz
RUN tar -xf ${PKG}-${VER}.tar.gz                                           \
 && cmake -S ${PKG}-${VER}                                                 \
    -B ${PKG}-${VER}-build                                                 \
    "-DCMAKE_INSTALL_PREFIX=/usr/local/"                                   \
    "-DCMAKE_BUILD_TYPE=Release" "-DBUILD_TESTS=OFF" "-DBUILD_TESTING=OFF" \
    "-DCMAKE_SKIP_INSTALL_RPATH=ON" "-DCMAKE_POSITION_INDEPENDENT_CODE=ON" \
    "-DBUILD_SHARED_LIBS=ON" "-DBUILD_STATIC_LIBS=ON"                      \
    "-DPAHO_BUILD_SHARED=ON"                                               \
    "-DPAHO_BUILD_STATIC=ON"                                               \
    "-DPAHO_WITH_SSL=ON"                                                   \
    "-DPAHO_ENABLE_TESTING=OFF"                                            \
 && make -C ${PKG}-${VER}-build -j$(nproc) install/strip                   \
 && rm -rf ${PKG}-${VER} ${PKG}-${VER}-build
# --------------------------------------------------------------------------
ARG PKG=nv-codec-headers
ARG VER=11.1.5.3
RUN wget -q --content-disposition                                          \
    https://github.com/FFmpeg/${PKG}/archive/refs/tags/n${VER}.tar.gz
RUN tar  -xf ${PKG}-n${VER}.tar.gz                                         \
 && make -C  ${PKG}-n${VER} install                                        \
 && rm   -rf ${PKG}-n${VER}
# --------------------------------------------------------------------------
ARG CPU_ARCH=unknown
ARG PKG=ffmpeg
ARG VER=4.4.4

RUN if [ "${CPU_ARCH}" = "amd64" ]; then                                   \
        wget -q --content-disposition                                      \
        https://ffmpeg.org/releases/${PKG}-${VER}.tar.xz;                  \
    fi
RUN if [ "${CPU_ARCH}" = "amd64" ]; then                                       \
        tar -xf ${PKG}-${VER}.tar.xz                                           \
        && pushd ${PKG}-${VER} >/dev/null                                      \
        && ./configure --prefix=/usr/local/                                    \
            --enable-shared              --enable-pic                          \
            --enable-cuda                --enable-nvenc         --enable-cuvid \
            --disable-htmlpages          --disable-manpages                    \
            --disable-podpages           --disable-txtpages                    \
            --disable-swresample                                               \
            --enable-hwaccel=h264_nvdec,hevc_nvdec,mpeg2_nvdec,mpeg4_nvdec     \
        && make -j$(nproc) install                                             \
        && popd >/dev/null                                                     \
        && rm -rf ${PKG}-${VER};                                               \
    fi
COPY ffmpeg-gcc9-orin-4.1.4.tar.gz /tmp
RUN if [ "${CPU_ARCH}" = "arm64" ]; then                                   \
        tar xf ffmpeg-gcc9-orin-4.1.4.tar.gz -C /usr/local;                \
    fi
# --------------------------------------------------------------------------
ARG PKG=gflags
ARG VER=2.2.2
RUN wget -q --content-disposition                                          \
    https://github.com/gflags/${PKG}/archive/refs/tags/v${VER}.tar.gz
RUN tar -xf ${PKG}-${VER}.tar.gz                                           \
 && cmake -S ${PKG}-${VER}                                                 \
    -B ${PKG}-${VER}-build                                                 \
    "-DCMAKE_INSTALL_PREFIX=/usr/local/"                                   \
    "-DCMAKE_BUILD_TYPE=Release" "-DBUILD_TESTS=OFF" "-DBUILD_TESTING=OFF" \
    "-DCMAKE_SKIP_INSTALL_RPATH=ON" "-DCMAKE_POSITION_INDEPENDENT_CODE=ON" \
    "-DBUILD_SHARED_LIBS=ON" "-DBUILD_STATIC_LIBS=ON"                      \
 && make -C ${PKG}-${VER}-build -j$(nproc) install/strip                   \
 && rm -rf ${PKG}-${VER}  ${PKG}-${VER}-build
# --------------------------------------------------------------------------
ARG PKG=glog
#ARG VER=0.6.0
ARG VER=0.4.0
RUN wget -q --content-disposition                                          \
    https://github.com/google/${PKG}/archive/refs/tags/v${VER}.tar.gz
RUN tar -xf ${PKG}-${VER}.tar.gz                                           \
 && cmake -S ${PKG}-${VER}                                                 \
    -B ${PKG}-${VER}-build                                                 \
    "-DCMAKE_INSTALL_PREFIX=/usr/local/"                                   \
    "-DCMAKE_BUILD_TYPE=Release" "-DBUILD_TESTS=OFF" "-DBUILD_TESTING=OFF" \
    "-DCMAKE_SKIP_INSTALL_RPATH=ON" "-DCMAKE_POSITION_INDEPENDENT_CODE=ON" \
    "-DBUILD_SHARED_LIBS=ON" "-DBUILD_STATIC_LIBS=ON"                      \
    "-DWITH_GTEST=OFF"                                                     \
    "-DWITH_THREADS=ON"                                                    \
    "-DWITH_UNWIND=ON"                                                     \
    "-DWITH_TLS=OFF"                                                       \
    "-DWITH_GFLAGS=OFF"                                                    \
 && make -C ${PKG}-${VER}-build -j$(nproc) install/strip                   \
 && rm -rf ${PKG}-${VER}  ${PKG}-${VER}-build
# --------------------------------------------------------------------------
ARG PKG=protobuf
ARG VER=3.14.0
RUN wget -q --content-disposition                                          \
    https://github.com/protocolbuffers/${PKG}/archive/refs/tags/v${VER}.tar.gz
RUN tar -xf ${PKG}-${VER}.tar.gz                                           \
 && cmake -S ${PKG}-${VER}/cmake                                           \
    -B ${PKG}-${VER}-build                                                 \
    "-DCMAKE_INSTALL_PREFIX=/usr/local/"                                   \
    "-DCMAKE_BUILD_TYPE=Release" "-DBUILD_TESTS=OFF" "-DBUILD_TESTING=OFF" \
    "-DCMAKE_SKIP_INSTALL_RPATH=ON" "-DCMAKE_POSITION_INDEPENDENT_CODE=ON" \
    "-DBUILD_SHARED_LIBS=ON" "-DBUILD_STATIC_LIBS=ON"                      \
    "-Dprotobuf_BUILD_SHARED_LIBS=ON"                                      \
    "-Dprotobuf_BUILD_TESTS=OFF"                                           \
 && make -C ${PKG}-${VER}-build -j$(nproc) install/strip                   \
 && pushd ${PKG}-${VER}/python >/dev/null                                  \
 && ldconfig                                                               \
 && ./setup.py install                                                     \
 && popd >/dev/null                                                        \
 && rm -rf ${PKG}-${VER}  ${PKG}-${VER}-build
# --------------------------------------------------------------------------
ARG PKG=opencv
ARG PKG_C=opencv_contrib
ARG VER=3.4.20
RUN wget -q --content-disposition                                          \
    https://github.com/opencv/${PKG}/archive/refs/tags/${VER}.tar.gz       \
    https://github.com/opencv/${PKG_C}/archive/refs/tags/${VER}.tar.gz
RUN tar -xf ${PKG}-${VER}.tar.gz                                           \
 && tar -xf ${PKG_C}-${VER}.tar.gz                                         \
 && cmake -S ${PKG}-${VER}                                                 \
    -B ${PKG}-${VER}-build                                                 \
    "-DCMAKE_INSTALL_PREFIX=/usr/local/"                                   \
    "-DCMAKE_BUILD_TYPE=Release" "-DBUILD_TESTS=OFF" "-DBUILD_TESTING=OFF" \
    "-DCMAKE_SKIP_INSTALL_RPATH=ON" "-DCMAKE_POSITION_INDEPENDENT_CODE=ON" \
    "-DBUILD_SHARED_LIBS=ON" "-DBUILD_STATIC_LIBS=ON"                      \
    "-DWITH_CUDA=ON"                                                       \
    "-DWITH_NVCUVID=ON"                                                    \
    "-DWITH_CUBLAS=ON"                                                     \
    "-DWITH_FFMPEG=ON"                                                     \
    "-DWITH_OPENGL=ON"                                                     \
    "-DWITH_V4L=ON"                                                        \
    "-DOPENCV_EXTRA_MODULES_PATH=${PKG_C}-${VER}/modules"                  \
    "-DCUDA_USE_STATIC_CUDA_RUNTIME=OFF"                                   \
    "-DBUILD_PERF_TESTS=OFF"                                               \
    "-DBUILD_CUDA_STUBS=OFF"                                               \
    "-DBUILD_EXAMPLES=OFF"                                                 \
    "-DBUILD_JAVA=OFF"                                                     \
    "-DBUILD_ANDROID_EXAMPLES=OFF"                                         \
    "-DINSTALL_PYTHON_EXAMPLES=OFF"                                        \
    "-DINSTALL_C_EXAMPLES=OFF"                                             \
    "-DBUILD_opencv_apps=OFF"                                              \
 && make -C ${PKG}-${VER}-build -j$(nproc) install/strip                   \
 && rm -rf ${PKG}-${VER} ${PKG_C}-${VER}  ${PKG}-${VER}-build
# --------------------------------------------------------------------------
RUN ldconfig

# ==========================================================================
FROM build_usr_local AS build_opt_local
# --------------------------------------------------------------------------
ARG PKG=Paddle
ARG VER=2.3.2
RUN wget -q --content-disposition                                          \
    https://github.com/PaddlePaddle/${PKG}/archive/refs/tags/v${VER}.tar.gz
RUN tar -xf ${PKG}-${VER}.tar.gz                                           \
    ;                                                                      \
    with_arm=OFF                                                           \
    ;                                                                      \
    case "$(uname -m)" in                                                  \
        x86_64) : ;;                                                       \
        aarch64) with_arm=ON ;;                                            \
        *)       echo "Unsupported architecture $(uname -m)" ; exit 1 ;;   \
    esac                                                                   \
    ;                                                                      \
    cmake -S ${PKG}-${VER}                                                 \
    -B ${PKG}-${VER}-build                                                 \
    "-DCMAKE_INSTALL_PREFIX=/usr/local/"                                   \
    "-DCMAKE_BUILD_TYPE=Release" "-DBUILD_TESTS=OFF" "-DBUILD_TESTING=OFF" \
    "-DCMAKE_SKIP_INSTALL_RPATH=ON" "-DCMAKE_POSITION_INDEPENDENT_CODE=ON" \
    "-DCUDA_USE_STATIC_CUDA_RUNTIME=OFF"                                   \
    "-DPYTHON_EXECUTABLE=/usr/bin/python3"                                 \
    "-DWITH_PYTHON=OFF"                                                    \
    "-DWITH_TENSORRT=ON"                                                   \
    "-DWITH_AVX=OFF"                                                       \
    "-DWITH_ARM=${with_arm}"                                               \
    "-DON_INFER=ON"                                                        \
    "-DCMAKE_CXX_FLAGS=-Wno-pessimizing-move -Wno-deprecated-copy"         \
  && make -C ${PKG}-${VER}-build -j$(nproc)                                \
  && rm  -rf ${PKG}-${VER}-build/paddle                                    \
  && :

RUN mkdir -p /opt/local                                                    \
 && mv  ${PKG}-${VER}-build/paddle_inference_install_dir/paddle            \
        /opt/local/                                                        \
 && find ${PKG}-${VER}-build/paddle_inference_install_dir/third_party/     \
          -name "lib*.so*" -exec mv  {} /opt/local/paddle/lib/ \;          \
 && :
# --------------------------------------------------------------------------

# ==========================================================================
FROM build_usr_local AS target
COPY --from=build_opt_local    /opt/local /opt/local

COPY bin /usr/local/bin/

ADD  fast-rtps-1.5.0-1.tar.xz  /usr/local
ADD  Hikvision-*.tar.xz        /opt/local/Hikvision
ADD  asn-wrapper.tar.xz        /opt/local/asn-wrapper
ADD  cyber-rt.tar.xz           /opt/local
RUN  mkdir -p /etc/v2x && echo RSCU_DEFAULT_SN > /etc/v2x/SN
RUN  ldconfig

