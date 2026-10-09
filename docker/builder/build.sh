#!/usr/bin/env bash
readonly PWD0="$(dirname "$(readlink -fn "$0")")"
#### ============================================================================
readonly IMAGE_NAME="airos/airos"
#### ============================================================================
set -e

function main {
  cd "${PWD0}"

  #### ==========================================================================
  local -a cmd=()
  if command -v buildah; then
    echo "Using buildah"
    cmd+=("buildah" "bud" "--format" "docker" "--layers" "--pull=false")
  elif command -v docker; then
    echo "Using docker"
    cmd+=("docker" "build")
  else
    echo "Neither buildah nor docker existed!"
    exit 1
  fi
  #### ==========================================================================
  local arc="$(uname -m)"
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
  local -ra targets=(
    base
    build_usr_local
    build_opt_local
    target
  )
  for target in "${targets[@]}"; do
    local tag="dev-${arc}-${target}"
    if [[ "${target}" == "target" ]]; then
      tag="dev-${arc}-$(date +%Y%m%d)"
    fi
    "${cmd[@]}" \
      --build-arg CPU_ARCH=${arc} \
      --ulimit nofile=102400:102400 \
      -f "dev.dockerfile" \
      -t "${IMAGE_NAME}:${tag}" \
      --target "${target}" \
      "dev-${arc}"
  done
}

main "$@"
