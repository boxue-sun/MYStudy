#!/usr/bin/env bash

TOP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
source "${TOP_DIR}/docker/scripts/utils/device.bashrc"

unset TOP_DIR

export HOST_ARCH="$(uname -m)"
export HOST_OS="$(uname -s)"

DOCKER_RUN_CMD="docker run"
USE_GPU_HOST=0

function determine_gpu_use_host() {
    if [[ "${HOST_ARCH}" == "aarch64" ]]; then
        if lsmod | grep -q "^nvgpu"; then
            USE_GPU_HOST=1
        fi
    elif [[ "${HOST_ARCH}" == "x86_64" ]]; then
        if [[ ! -x "$(command -v nvidia-smi)" ]]; then
            warning "No nvidia-smi found. CPU will be used"
        elif [[ -z "$(nvidia-smi)" ]]; then
            warning "No GPU device found. CPU will be used."
        else
            USE_GPU_HOST=1
        fi
    else
        error "Unsupported CPU architecture: ${HOST_ARCH}"
        exit 1
    fi

    local nv_docker_doc="https://github.com/NVIDIA/nvidia-docker/blob/master/README.md"
    if [[ "${USE_GPU_HOST}" -eq 1 ]]; then
        if [[ -x "$(which nvidia-container-toolkit)" ]]; then
            local docker_version
            docker_version="$(docker version --format '{{.Server.Version}}')"
            if dpkg --compare-versions "${docker_version}" "ge" "19.03"; then
                DOCKER_RUN_CMD="docker run --gpus all --runtime=nvidia"
            else
                warning "Please upgrade to docker-ce 19.03+ to access GPU from container."
                USE_GPU_HOST=0
            fi
        elif [[ -x "$(which nvidia-docker)" ]]; then
            DOCKER_RUN_CMD="nvidia-docker run"
        else
            USE_GPU_HOST=0
            warning "Cannot access GPU from within container. Please install latest Docker" \
                "and NVIDIA Container Toolkit as described by: "
            warning "  ${nv_docker_doc}"
        fi
    fi
}

function remove_container_if_exists() {
    local container="$1"
    if docker ps -a --format '{{.Names}}' | grep -q "${container}"; then
        info "Removing existing ${PROJECT_NAME} container: ${container}"
        docker stop "${container}" >/dev/null
        docker rm -v -f "${container}" 2>/dev/null
    fi
}

function postrun_start_user() {
    local container="$1"
    if [ "${USER}" != "root" ]; then
        docker exec -u root "${container}" \
            bash -c './docker/scripts/utils/docker_start_user.sh'
    fi
}

function stop_all_apollo_containers() {
    local force="$1"
    local running_containers
    running_containers="$(docker ps -a --format '{{.Names}}')"
    for container in ${running_containers[*]}; do
        if [[ "${container}" =~ apollo_.*_${USER} ]]; then
            #printf %-*s 70 "Now stop container: ${container} ..."
            #printf "\033[32m[DONE]\033[0m\n"
            #printf "\033[31m[FAILED]\033[0m\n"
            info "Now stop container ${container} ..."
            if docker stop "${container}" >/dev/null; then
                if [[ "${force}" == "-f" || "${force}" == "--force" ]]; then
                    docker rm -f "${container}" 2>/dev/null
                fi
                info "Done."
            else
                warning "Failed."
            fi
        fi
    done
}

# Check whether user has agreed license agreement
function check_agreement() {
    local agreement_record="${HOME}/.airos_agreement.txt"
    if [[ -e "${agreement_record}" ]]; then
        return 0
    fi
    local agreement_file
    agreement_file="${DEVICE_ROOT_DIR}/docker/scripts/utils/AGREEMENT.txt"
    if [[ ! -f "${agreement_file}" ]]; then
        error "AGREEMENT ${agreement_file} does not exist."
        exit 1
    fi

    cat "${agreement_file}"
    local tip="Type 'y' or 'Y' to agree to the license agreement above, \
or type any other key to exit:"

    echo -n "${tip}"
    local answer="$(read_one_char_from_stdin)"
    echo

    if [[ "${answer}" != "y" ]]; then
        exit 1
    fi

    cp -f "${agreement_file}" "${agreement_record}"
    echo "${tip}" >> "${agreement_record}"
    echo "${user_agreed}" >> "${agreement_record}"
}

export -f determine_gpu_use_host
export -f stop_all_apollo_containers remove_container_if_exists
export -f check_agreement
export USE_GPU_HOST
export DOCKER_RUN_CMD
