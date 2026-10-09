#! /usr/bin/env bash
set -e
#### ============================================================================
function main {
    readonly IMAGE_NAME="registry.baidubce.com/zhiluos/airos"
    DEV_SDK_PATH="/airos/sdk"
    RUNTIME_PATH="/home/airos"
    PKG_PATH="/airos/package/package"
    AIROS_TMP_PATH="/tmp/airos"

    TOP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../" && pwd -P)"
    PROJECT_NAME=$(basename ${TOP_DIR})
    DOCKER_USER="${USER}"
    DEV_CONTAINER="${PROJECT_NAME}_dev_${USER}"

    if [[ "$@" == "base" ]]; then
        DEV_CONTAINER="airos_framework_img"
    fi
    
    if [ -d "${AIROS_TMP_PATH}" ]; then
        rm -rf "${AIROS_TMP_PATH}"
    fi
    mkdir -p "${AIROS_TMP_PATH}"

    docker cp "${DEV_CONTAINER}:${DEV_SDK_PATH}" "${AIROS_TMP_PATH}"
    docker cp "${DEV_CONTAINER}:${RUNTIME_PATH}" "${AIROS_TMP_PATH}"
    docker cp "${DEV_CONTAINER}:${PKG_PATH}" "${AIROS_TMP_PATH}"

    #### ==========================================================================
    arc="$(uname -m)"
    case "${arc}" in
    "aarch64" | "arm64")
    arc="arm64"
    ;;
    "x86_64" | "amd64")
    arc="amd64"
    ;;
    *)
    echo "Unknown arch: ${arc}"
    exit 1
    ;;
    esac

    #### ==========================================================================
    tag="dev-${arc}-$(date +%Y%m%d)-sdk"
    local dockerfile="sdk_${arc}.dockerfile"

    if [[ "$@" == "base" ]]; then
        dockerfile="sdk_base.dockerfile"
        tag="base-${arc}-$(date +%Y%m%d)-sdk"
    fi
    docker build \
        --ulimit nofile=102400:102400 \
        -f "${TOP_DIR}/docker/builder/${dockerfile}" \
        -t "${IMAGE_NAME}:${tag}" \
        /tmp/airos
}

main "$@"
