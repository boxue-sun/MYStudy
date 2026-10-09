#! /usr/bin/env bash
set -e

DEV_SDK_PATH="/airos/sdk"
AIROS_SDK_PATH="${DEV_SDK_PATH}/airos"
AIROS_3RD_PATH="${DEV_SDK_PATH}/third_party"
SDK_INCLUDES="${AIROS_SDK_PATH}/include"
SDK_LIBS="${AIROS_SDK_PATH}/lib"
[ -d "${SDK_INCLUDES}" ] || mkdir -p ${SDK_INCLUDES}
[ -d "${SDK_LIBS}" ] || mkdir -p ${SDK_LIBS}

# make include
# air service
rsync -R air_service/framework/perception-camera/interface/base_camera_perception.h ${SDK_INCLUDES}
rsync -R air_service/framework/perception-fusion/interface/base_perception_fusion.h ${SDK_INCLUDES}
rsync -R air_service/framework/perception-lidar/interface/base_lidar_perception.h ${SDK_INCLUDES}
# app
find app/framework/interface -name "*.h" -exec rsync -R {} ${SDK_INCLUDES} \;
# base
find base -name "*.h" -exec rsync -R {} ${SDK_INCLUDES} \;
# middleware
find middleware/device_service/framework/camera  -name "*.h" -exec rsync -R {} ${SDK_INCLUDES} \;

# make lib
find bazel-bin/air_service/framework/proto -maxdepth 1 -name "*.so" -exec cp -rf {} ${SDK_LIBS} \;
find bazel-bin/base -maxdepth 1 -name "*.so" -exec cp -rf {} ${SDK_LIBS} \;
find bazel-bin/base/device_connect/proto -maxdepth 1 -name "*.so" -exec cp -rf {} ${SDK_LIBS} \;
find bazel-bin/app/framework -maxdepth 2 -name "*.so" -exec cp -rf {} ${SDK_LIBS} \;

# make proto
SERVICE_PROTO_PATH=air_service/framework/proto
SERVICE_FUSION_PROTO_PATH=air_service/framework/perception-fusion/proto
APP_PROTO_PATH=app/framework/proto
DEVICE_PROTO_PATH=base/device_connect/proto
mkdir -p ${SDK_INCLUDES}/${SERVICE_PROTO_PATH}
mkdir -p ${SDK_INCLUDES}/${SERVICE_FUSION_PROTO_PATH}
mkdir -p ${SDK_INCLUDES}/${APP_PROTO_PATH} 
mkdir -p ${SDK_INCLUDES}/${DEVICE_PROTO_PATH} 

find bazel-bin/${SERVICE_PROTO_PATH} -name "*.pb.h" -exec cp {} ${SDK_INCLUDES}/${SERVICE_PROTO_PATH}  \;
find bazel-bin/${SERVICE_FUSION_PROTO_PATH} -name "*.pb.h" -exec cp {} ${SDK_INCLUDES}/${SERVICE_FUSION_PROTO_PATH}  \;
find bazel-bin/${APP_PROTO_PATH} -name "*.pb.h" -exec cp {} ${SDK_INCLUDES}/${APP_PROTO_PATH} \;
find bazel-bin/${DEVICE_PROTO_PATH} -name "*.pb.h" -exec cp {} ${SDK_INCLUDES}/${DEVICE_PROTO_PATH}  \;

# make BUILD
cat > ${AIROS_SDK_PATH}/BUILD <<EOF
load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "airos",
    includes = ["include"],
    hdrs = glob(["include/**/*"]),
    srcs = glob(["lib/**/lib*.so*"]),
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)

EOF
cp -rf third_party/ ${AIROS_3RD_PATH}
touch ${DEV_SDK_PATH}/BUILD
