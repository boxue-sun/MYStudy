#!/usr/bin/env bash

TOP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../" && pwd -P)"
PROJECT_NAME=$(basename ${TOP_DIR})

DOCKER_USER="${USER}"
DEV_CONTAINER="${PROJECT_NAME}_dev_${USER}"
SDK_CONTAINER="${PROJECT_NAME}_sdk_${USER}"

xhost +local:root 1>/dev/null 2>&1

CONTAINER=${DEV_CONTAINER}
case "$1" in
"sdk")
    CONTAINER="${SDK_CONTAINER}"
    ;;
*)
    CONTAINER="${DEV_CONTAINER}"
    ;;
esac

docker exec \
    -u "${DOCKER_USER}" \
    -it "${CONTAINER}" \
    /bin/bash

xhost -local:root 1>/dev/null 2>&1
