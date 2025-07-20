#!/bin/bash

CONFIGURE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
CONFIGURE_SCRIPT_PATH="${CONFIGURE_DIR}/$(basename "$0")"
CONFIGURE_CMAKE_PATH="${CONFIGURE_DIR}/../../alkos/CMakeLists.txt"
CONFIGURE_TOOLCHAIN_DIR="${CONFIGURE_DIR}/../../alkos/toolchains"

source "${CONFIGURE_DIR}/../utils/pretty_print.bash"
source "${CONFIGURE_DIR}/../utils/helpers.bash"

declare -A CONFIGURE_TOOLCHAINS=(
  ["x86_64"]="${CONFIGURE_TOOLCHAIN_DIR}/x86_64-toolchain.cmake"
)

declare -A CONFIGURE_FLAGS=(
  ["x86_64"]="${CONFIGURE_TOOLCHAIN_DIR}/x86_64-flags.cmake"
)

declare -A CONFIGURE_BUILD_TYPES_DESC=(
  ["debug_qemu_tests"]="Build with full QEMU debugging capabilities running test framework, built in debug."
  ["release_qemu_tests"]="Build with full QEMU debugging capabilities running test framework, built in release."
  ["debug_qemu"]="Build with full QEMU debugging capabilities, built in debug."
  ["release_qemu"]="Build with full QEMU debugging capabilities, built in release."
  ["debug_qemu_no_traces"]="Build with full QEMU debugging capabilities, built in debug, without traces."
  ["release_qemu_no_traces"]="Build with full QEMU debugging capabilities, built in release, without traces."
  ["debug"]="Build in debug. Capable of running on real hardware."
  ["release"]="Build in release. Capable of running on real hardware."
)

declare -A CONFIGURE_CMAKE_BUILD_TYPES=(
  ["debug_qemu_tests"]="Debug"
  ["release_qemu_tests"]="Release"
  ["debug_qemu"]="Debug"
  ["release_qemu"]="Release"
  ["debug_qemu_no_traces"]="Debug"
  ["release_qemu_no_traces"]="Release"
  ["debug"]="Debug"
  ["release"]="Release"
)

declare -A CONFIGURE_BUILD_TYPES_DEFINES=(
    [debug_qemu_tests]="ENABLE_TESTS USE_DEBUG_OUTPUT USE_DEBUG_TRACES"
    [release_qemu_tests]="ENABLE_TESTS USE_DEBUG_OUTPUT USE_DEBUG_TRACES"
    [debug_qemu]="USE_DEBUG_OUTPUT USE_DEBUG_TRACES"
    [release_qemu]="USE_DEBUG_OUTPUT USE_DEBUG_TRACES"
    [debug_qemu_no_traces]="USE_DEBUG_OUTPUT"
    [release_qemu_no_traces]="USE_DEBUG_OUTPUT"
    [debug]=""
    [release]=""
)

CONFIGURE_BUILD_DIR=""
CONFIGURE_TOOL_BINARIES_DIR=""
CONFIGURE_ARCH=""
CONFIGURE_BUILD_TYPE=""
CONFIGURE_VERBOSE=false

help() {
  echo "${CONFIGURE_SCRIPT_PATH} <arch> <build> [-b <BUILD_DIR>] [-t <TOOL_DIR>] [--verbose | -v]"
  echo "Where:"
  echo "<arch> - MANDATORY - architecture to build for. Supported are listed below."
  echo "<build> - MANDATORY - build type to configure. Supported are listed below."
  echo "-b <BUILD_DIR> - directory to store build files. Default is 'build' in repo parent dir."
  echo "-t <TOOL_DIR> - directory to store tool binaries. Default is 'tools' in repo parent dir."
  echo "--verbose | -v - flag to enable verbose output"

  echo ""
  echo "Supported architectures:"
  for arch in "${!CONFIGURE_TOOLCHAINS[@]}"; do
    echo "  ${arch}"
  done

  echo ""
  echo "Supported build types:"
  for build in "${!CONFIGURE_BUILD_TYPES_DESC[@]}"; do
    echo "  ${build} - ${CONFIGURE_BUILD_TYPES_DESC[${build}]}"
  done
}

parse_args() {
  while [[ $# -gt 0 ]]; do
    case $1 in
      -h|--help)
        help
        exit 0
        ;;
      -b)
        CONFIGURE_BUILD_DIR="$2"
        shift 2
        ;;
      -t)
        CONFIGURE_TOOL_BINARIES_DIR="$2"
        shift 2
        ;;
      -v|--verbose)
        CONFIGURE_VERBOSE=true
        shift
        ;;
      -*)
        dump_error "Unknown argument: $1"
        exit 1
        ;;
      *)
        if [[ -z "$CONFIGURE_ARCH" ]]; then
          CONFIGURE_ARCH="$1"
        elif [[ -z "$CONFIGURE_BUILD_TYPE" ]]; then
          CONFIGURE_BUILD_TYPE="$1"
        else
          dump_error "Unknown argument: $1"
          exit 1
        fi
        shift
        ;;
    esac
  done
}

process_args() {
  if [[ -z "$CONFIGURE_ARCH" ]]; then
    dump_error "Architecture not specified. Use -h for help."
    exit 1
  fi

  if [[ -z "$CONFIGURE_BUILD_TYPE" ]]; then
    dump_error "Build type not specified. Use -h for help."
    exit 1
  fi

  if [[ -z "${CONFIGURE_TOOLCHAINS[$CONFIGURE_ARCH]}" ]]; then
    dump_error "Unsupported architecture: $CONFIGURE_ARCH. Use -h for help."
    exit 1
  fi

  if [[ -z "${CONFIGURE_BUILD_TYPES_DESC[$CONFIGURE_BUILD_TYPE]}" ]]; then
    dump_error "Unsupported build type: $CONFIGURE_BUILD_TYPE. Use -h for help."
    exit 1
  fi

  # Set default values if not provided
  [[ -z "$CONFIGURE_BUILD_DIR" ]] && CONFIGURE_BUILD_DIR="${CONFIGURE_DIR}/../../build"
  [[ -z "$CONFIGURE_TOOL_BINARIES_DIR" ]] && CONFIGURE_TOOL_BINARIES_DIR="${CONFIGURE_DIR}/../../tools"
}

run() {
  pretty_info "Configuring AlkOS build..."
  pretty_info "Architecture: $CONFIGURE_ARCH"
  pretty_info "Build type: $CONFIGURE_BUILD_TYPE"
  pretty_info "Build directory: $CONFIGURE_BUILD_DIR"
  pretty_info "Tool binaries directory: $CONFIGURE_TOOL_BINARIES_DIR"
  pretty_info "Verbose mode: $CONFIGURE_VERBOSE"

  pretty_info "Creating cmake configuration files..."
  # prepare conf.generated.cmake
  local conf_cmake="${CONFIGURE_DIR}/../config/conf.generated.cmake"

  echo "# This file is generated by configure script. Do not edit manually." > "$conf_cmake"
  echo "" >> "$conf_cmake"
  echo "message(STATUS \"Configuring AlkOS build...\")" >> "$conf_cmake"
  echo "set(CMAKE_TOOLCHAIN_FILE \"${CONFIGURE_TOOLCHAINS[$CONFIGURE_ARCH]}\")" >> "$conf_cmake"
  echo "set(CMAKE_FLAGS_FILE \"${CONFIGURE_FLAGS[$CONFIGURE_ARCH]}\")" >> "$conf_cmake"
  echo "set(CMAKE_BUILD_TYPE \"${CONFIGURE_CMAKE_BUILD_TYPES[$CONFIGURE_BUILD_TYPE]}\")" >> "$conf_cmake"
  echo "set(TOOL_BINARIES_DIR \"${CONFIGURE_TOOL_BINARIES_DIR}\")" >> "$conf_cmake"
  echo "set(CMAKE_BUILD_DIR \"${CONFIGURE_BUILD_DIR}\")" >> "$conf_cmake"

  local cmake_arg
  for cmake_arg in ${CONFIGURE_BUILD_TYPES_DEFINES[$CONFIGURE_BUILD_TYPE]}; do
      echo "set(${cmake_arg} ON)" >> "$conf_cmake"
  done

  pretty_info "Preparing build directory..."
  base_runner "Failed to create build directory" "${CONFIGURE_VERBOSE}" mkdir -p "${CONFIGURE_BUILD_DIR}"

  # let the cmake generate bash.conf and build files
  pretty_info "Running cmake..."
  base_runner "Failed to run cmake" "${CONFIGURE_VERBOSE}" cmake "${CONFIGURE_CMAKE_PATH}" -B "${CONFIGURE_BUILD_DIR}/alkos" \
        -G "Unix Makefiles"
}

main() {
  parse_args "$@"
  process_args
  run
}

main "$@"
