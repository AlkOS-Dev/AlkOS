#!/bin/bash

# script dirs
INSTALL_TOOLCHAIN_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
INSTALL_TOOLCHAIN_PATH="${INSTALL_TOOLCHAIN_DIR}/$(basename "$0")"

# script dirs
INSTALL_TOOLCHAIN_BUILD_SCRIPT_PATH="${INSTALL_TOOLCHAIN_DIR}/build_cross_compile.bash"

# sources
source "${INSTALL_TOOLCHAIN_DIR}/../utils/pretty_print.bash"
source "${INSTALL_TOOLCHAIN_DIR}/../utils/helpers.bash"

# global conf
declare -A INSTALL_TOOCHAIN_ARCH_DICT=(
  ["x86_64"]="x86_64-elf i386-elf"
)

# flags
INSTALL_TOOLCHAIN_ARCH=""
INSTALL_TOOLCHAIN_INSTALL_DIR=""
INSTALL_TOOLCHAIN_BUILD_DIR=""
INSTALL_TOOLCHAIN_VERBOSE=false

help() {
  echo "SCRIPT <install_dir> <build_dir> <arch> [--verbose | -v]"
  echo "Where:"
  echo "install_dir - path to the directory where the toolchain will be installed"
  echo "build_dir   - path to the directory where the build files will be located"
  echo "arch       - architecture to build the toolchain for"
  echo "--verbose   | -v - flag to enable verbose output"
  echo ""
  echo "Supported architectures:"
  for arch in "${!INSTALL_TOOCHAIN_ARCH_DICT[@]}"; do
    echo "  ${arch}"
  done
}

parse_args() {
  if [ $# -lt 3 ]; then
    echo "Error: Missing required arguments."
    help
    exit 1
  fi

  INSTALL_TOOLCHAIN_INSTALL_DIR="$1"
  INSTALL_TOOLCHAIN_BUILD_DIR="$2"
  INSTALL_TOOLCHAIN_ARCH="$3"  # Splitting the space-separated list of architectures

  shift 3  # Move past the first three arguments

  while [[ $# -gt 0 ]]; do
    case $1 in
      -h|--help)
        help
        exit 0
        ;;
      -v|--verbose)
        INSTALL_TOOLCHAIN_VERBOSE=true
        shift
        ;;
      *)
        dump_error "Unknown argument: $1"
        ;;
    esac
  done
}

process_args() {
  # Validate architecture is supported
  if [[ -z "${INSTALL_TOOCHAIN_ARCH_DICT[$INSTALL_TOOLCHAIN_ARCH]}" ]]; then
    dump_error "Unsupported architecture: $INSTALL_TOOLCHAIN_ARCH"
    exit 1
  fi

  # Set verbose flag
  if [[ "${INSTALL_TOOLCHAIN_VERBOSE}" == "true" ]]; then
    INSTALL_TOOLCHAIN_VERBOSE_FLAG="--verbose"
  else
    INSTALL_TOOLCHAIN_VERBOSE_FLAG=""
  fi
}

main() {
  parse_args "$@"
  process_args

  # Ensure directories exists
  base_runner "Failed to create install directory" true mkdir -p "${INSTALL_TOOLCHAIN_INSTALL_DIR}"
  base_runner "Failed to create build directory" true mkdir -p "${INSTALL_TOOLCHAIN_BUILD_DIR}"

  pretty_info "Installing cross-compile toolchain"
  for arch in ${INSTALL_TOOCHAIN_ARCH_DICT[${INSTALL_TOOLCHAIN_ARCH}]}; do
    pretty_info "Installing for ${arch}"
    base_runner "Failed to clean build directory" true "${INSTALL_TOOLCHAIN_CLEAN_BUILD_SCRIPT_PATH}" ${INSTALL_TOOLCHAIN_VERBOSE_FLAG}
    # Spawn a subshell to run the build script (to avoid polluting the current shell)
    (
      base_runner "Failed to install cross-compile toolchain" true "${INSTALL_TOOLCHAIN_BUILD_SCRIPT_PATH}" --install \
        toolchain "${INSTALL_TOOLCHAIN_VERBOSE_FLAG}"   \
          -t "${INSTALL_TOOLCHAIN_INSTALL_DIR}/${arch}" \
          -b "${INSTALL_TOOLCHAIN_BUILD_DIR}/${arch}"   \
          -c "${arch}"

      base_runner "Failed to remove build directory" true rm -rf "${INSTALL_TOOLCHAIN_BUILD_DIR}/${arch}"

      exit 0
    ) 2>&1 || exit 1
  done
}

main "$@"
