#!/usr/bin/env bash

CURR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
source "${CURR_DIR}/docker_base.sh"

CACHE_ROOT_DIR="${DEVICE_ROOT_DIR}/.cache"

DEV_CONTAINER="${PROJECT_NAME}_dev_${USER}"
SDK_CONTAINER="${PROJECT_NAME}_sdk_${USER}"

DOCKER_ROOT="/home/airos"
USER_AGREED="no"
USER_SPECIFIED_MAPS=
MAP_VOLUMES_CONF=
OTHER_VOLUMES_CONF=
# =================================================================================================
# Image configure
readonly CONFIG_DOCKER_IMG_REPO_AARCH64="cictci/airos"
# readonly CONFIG_DOCKER_IMG_REPO_AARCH64="registry.baidubce.com/zhiluos/airos"
readonly CONFIG_DOCKER_IMG_REPO="registry.baidubce.com/zhiluos/airos"
readonly CONFIG_DOCKER_IMG_VERSION_X86_64="dev-amd64-20231208"
readonly CONFIG_DOCKER_IMG_VERSION_AARCH64="dev-arm64-20240710-inter"
# readonly CONFIG_DOCKER_IMG_VERSION_AARCH64="dev-arm64-20240605"
# readonly CONFIG_DOCKER_IMG_VERSION_AARCH64="dev-arm64-20240625"
readonly CONFIG_DOCKER_SDK_IMG_VERSION_X86_64="dev-amd64-20231208-sdk"
readonly CONFIG_DOCKER_SDK_IMG_VERSION_AARCH64="dev-arm64-20240125-sdk"
# =================================================================================================
# Force pull configure: force pull if not empty
readonly CONFIG_DOCKER_FORCE_PULL=
readonly CONFIG_DOCKER_SHM_SIZE="2G"
# =================================================================================================

function docker_pull() {
    local img="$1"
    if [[ -z "${CONFIG_DOCKER_FORCE_PULL}" ]]; then
        info "Start docker container based on local image : ${img}"
        if docker images --format "{{.Repository}}:{{.Tag}}" | grep -q "${img}"; then
            info "Local image ${img} found and will be used."
            return
        fi
        warning "Image ${img} not found locally although local mode enabled. Trying to pull from remote registry."
    fi
    info "Start pulling docker image ${img} ..."
    if ! docker pull "${img}"; then
        error "Failed to pull docker image : ${img}"
        exit 1
    fi
}

function docker_restart_volume() {
    local volume="$1"
    local image="$2"
    local path="$3"
    info "Create volume ${volume} from image: ${image}"
    docker_pull "${image}"
    docker volume rm "${volume}" >/dev/null 2>&1
    docker run -v "${volume}":"${path}" --rm "${image}" true
}

function main_dev() {
    local arch ver
    arch="$(uname -m)"
    local img
    case "$arch" in
    "x86_64")
        ver="${CONFIG_DOCKER_IMG_VERSION_X86_64}"
        img="${CONFIG_DOCKER_IMG_REPO}:${ver}"
        ;;
    "aarch64")
        ver="${CONFIG_DOCKER_IMG_VERSION_AARCH64}"
        img="${CONFIG_DOCKER_IMG_REPO_AARCH64}:${ver}"
        ;;
    *)
        error "Unsupported target architecture: $arch."
        ;;
    esac

    if ! docker_pull "${img}"; then
       error "Failed to pull docker image ${img}"
       exit 1
    fi

    info "Remove existing airos Development container ..."
    remove_container_if_exists ${DEV_CONTAINER}

    info "Determine whether host GPU is available ..."
    determine_gpu_use_host
    info "USE_GPU_HOST: ${USE_GPU_HOST}"

    mkdir -p "${CACHE_ROOT_DIR}"

    local display="${DISPLAY:-:0}"
    local user="${USER}"
    local uid gid group
    uid="$(id -u)"
    group="$(id -g -n)"
    gid="$(id -g)"

    set -x
    local dev_inside="in-docker"
    local -a docker_args=(
        -v "$DEVICE_ROOT_DIR:/${DOCKER_ROOT}"
        -v "/media:/media"
        -v "/tmp/.X11-unix:/tmp/.X11-unix:rw"
        -v "/etc/localtime:/etc/localtime:ro"
        -v "/usr/src:/usr/src:ro"
        -v "/lib/modules:/lib/modules:ro"
        -v "/sys/kernel/debug:/sys/kernel/debug:ro"
        #        "-v" "/dev:/dev"
        #        "-v" "/etc/v2x/:/etc/v2x"
        #        "-v" "/usr/local/var/db/openedge/calibration/:/calibration"
        #        "-v" "/xlog/log:/xlog/log"
        #        "-v" "/dev/null:/dev/raw1394"

        --shm-size "${CONFIG_DOCKER_SHM_SIZE}"

        -e USER="${user}"
        -e DOCKER_USER="${user}"
        -e DOCKER_USER_ID="${uid}"
        -e DOCKER_GRP="${group}"
        -e DOCKER_GRP_ID="${gid}"
        -e DOCKER_IMG="${img}"

        -e DISPLAY="${display}"
        -e USE_GPU_HOST="${USE_GPU_HOST}"
        -e "NVIDIA_VISIBLE_DEVICES=all"
        -e "NVIDIA_DRIVER_CAPABILITIES=compute,video,graphics,utility"

        --add-host "${dev_inside}:127.0.0.1"
        --add-host "$(hostname):127.0.0.1"
        --hostname "${dev_inside}"

        -w "/${DOCKER_ROOT}"

        --privileged
    )

    info "Starting Docker container \"${DEV_CONTAINER}\" ..."

 ${DOCKER_RUN_CMD} -itd \
        --name "${DEV_CONTAINER}" \
        --net host \
        "${docker_args[@]}" \
        "${img}" \
        /bin/bash

    if [ $? -ne 0 ]; then
        error "Failed to start docker container \"${DEV_CONTAINER}\" based on image: ${img}"
        exit 1
    fi
    set +x

    postrun_start_user "${DEV_CONTAINER}"

    ok "Congratulations! You have successfully finished setting up airos Dev Environment."
    ok "To login into the newly created ${DEV_CONTAINER} container, please run the following command:"
    ok "  bash docker/scripts/docker_into.sh"
    ok "Enjoy!"
}

function main_sdk() {
    local arch ver
    arch="$(uname -m)"
    case "$arch" in
    "x86_64")
        ver="${CONFIG_DOCKER_SDK_IMG_VERSION_X86_64}"
        ;;
    "aarch64")
        ver="${CONFIG_DOCKER_SDK_IMG_VERSION_AARCH64}"
        ;;
    *)
        error "Unsupported target architecture: $arch."
        ;;
    esac
    local img="${CONFIG_DOCKER_IMG_REPO}:${ver}"
    if ! docker_pull "${img}"; then
        error "Failed to pull docker image ${img}"
        exit 1
    fi

    info "Remove existing airos sdk container ..."
    remove_container_if_exists ${SDK_CONTAINER}

    info "Determine whether host GPU is available ..."
    determine_gpu_use_host
    info "USE_GPU_HOST: ${USE_GPU_HOST}"

    mkdir -p "${CACHE_ROOT_DIR}"
    mkdir -p "${DEVICE_ROOT_DIR}/projs"


    local display="${DISPLAY:-:0}"
    local user="${USER}"
    local uid gid group
    uid="$(id -u)"
    group="$(id -g -n)"
    gid="$(id -g)"

    set -x
    local dev_inside="in-docker"
    local -a docker_args=(
        -v "${DEVICE_ROOT_DIR}/docker/scripts/utils/docker_start_user.sh:/${DOCKER_ROOT}/docker/scripts/utils/docker_start_user.sh"
        -v "${DEVICE_ROOT_DIR}/projs:/home/airos/projs"
        -v "/media:/media"
        -v "/tmp/.X11-unix:/tmp/.X11-unix:rw"
        -v "/etc/localtime:/etc/localtime:ro"
        -v "/usr/src:/usr/src:ro"
        -v "/lib/modules:/lib/modules:ro"
        #        "-v" "/dev:/dev"
        #        "-v" "/etc/v2x/:/etc/v2x"
        #        "-v" "/usr/local/var/db/openedge/calibration/:/calibration"
        #        "-v" "/xlog/log:/xlog/log"
        #        "-v" "/dev/null:/dev/raw1394"

        --shm-size "${CONFIG_DOCKER_SHM_SIZE}"

        -e USER="${user}"
        -e DOCKER_USER="${user}"
        -e DOCKER_USER_ID="${uid}"
        -e DOCKER_GRP="${group}"
        -e DOCKER_GRP_ID="${gid}"
        -e DOCKER_IMG="${img}"

        -e DISPLAY="${display}"
        -e USE_GPU_HOST="${USE_GPU_HOST}"
        -e "NVIDIA_VISIBLE_DEVICES=all"
        -e "NVIDIA_DRIVER_CAPABILITIES=compute,video,graphics,utility"

        --add-host "${dev_inside}:127.0.0.1"
        --add-host "$(hostname):127.0.0.1"
        --hostname "${dev_inside}"

        -w "/${DOCKER_ROOT}"

        --privileged
    )

    info "Starting Docker container \"${SDK_CONTAINER}\" ..."

 ${DOCKER_RUN_CMD} -itd \
        --name "${SDK_CONTAINER}" \
        --net host \
        "${docker_args[@]}" \
        "${img}" \
        /bin/bash

    if [ $? -ne 0 ]; then
        error "Failed to start docker container \"${SDK_CONTAINER}\" based on image: ${img}"
        exit 1
    fi
    set +x

    postrun_start_user "${SDK_CONTAINER}"

    ok "Congratulations! You have successfully finished setting up airos sdk Environment."
    ok "To login into the newly created ${SDK_CONTAINER} container, please run the following command:"
    ok "  bash docker/scripts/docker_into.sh sdk"
    ok "Enjoy!"
}

function main() {
    check_agreement
    # -------------------------------------------------------
    local os
    os="$(uname -s)"
    case "$os" in
    "Linux")
        :
        ;;
    *)
        warning "Unexpected OS: $os"
        exit 1
        ;;
    esac
    # -------------------------------------------------------
    case "$1" in
    "sdk")
        main_sdk
        ;;
    *)
        main_dev
        ;;
    esac
}

main "$@"
