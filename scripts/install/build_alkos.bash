#!/bin/bash

# Get the script's directory and paths
BUILD_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
BUILD_SCRIPT_PATH="${BUILD_SCRIPT_DIR}/$(basename "$0")"
BUILD_SCRIPT_SOURCE_DIR="${BUILD_SCRIPT_DIR}/../.."

BUILD_SCRIPT_CONF_PATH="${BUILD_SCRIPT_SOURCE_DIR}/config/conf.generated.bash"

# Source helper scripts
source "${BUILD_SCRIPT_SOURCE_DIR}/scripts/utils/helpers.bash"
source "${BUILD_SCRIPT_SOURCE_DIR}/scripts/utils/pretty_print.bash"

# Default values for script flags
BUILD_SCRIPT_VERBOSE=false
BUILD_SCRIPT_CMAKE_CONF_PATH=""

# Help message
help() {
  echo "${BUILD_SCRIPT_PATH} <cmake_conf_dir> [--verbose | -v]"
  echo "Where:"
  echo "  cmake_conf_dir - path to the directory containing the CMake configuration files"
  echo "  --verbose | -v - flag to enable verbose output"
}

# Parse command-line arguments
parse_args() {
  if [[ $# -lt 1 ]]; then
    echo "Error: Missing required argument <cmake_conf_dir>"
    help
    exit 1
  fi

  # First argument is cmake_conf_dir
  BUILD_SCRIPT_CMAKE_CONF_PATH="$1"
  shift

  # Parse additional flags
  while [[ $# -gt 0 ]]; do
    case $1 in
      -h|--help)
        help
        exit 0
        ;;
      -v|--verbose)
        BUILD_SCRIPT_VERBOSE=true
        shift
        ;;
      *)
        echo "Unknown argument: $1"
        help
        exit 1
        ;;
    esac
  done
}

# Validate the provided arguments
process_args() {
  if [ -z "${BUILD_SCRIPT_CMAKE_CONF_PATH}" ]; then
    dump_error "The <cmake_conf_dir> argument is missing!"
    exit 1
  fi

  if [ ! -d "${BUILD_SCRIPT_CMAKE_CONF_PATH}" ]; then
    dump_error "The specified CMake configuration directory does not exist: ${BUILD_SCRIPT_CMAKE_CONF_PATH}"
    exit 1
  fi
}

# Main function
main() {
  parse_args "$@"
  process_args

  pretty_info "Building AlkOS from directory: ${BUILD_SCRIPT_CMAKE_CONF_PATH}"

  base_runner "Failed to build AlkOS" "${BUILD_SCRIPT_VERBOSE}" cmake --build "${BUILD_SCRIPT_CMAKE_CONF_PATH}" --target iso
  pretty_success "AlkOS built successfully"
}

# Run the script
main "$@"
