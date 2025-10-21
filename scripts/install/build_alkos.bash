#!/bin/bash

# Get the script's directory and paths
BUILD_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
BUILD_SCRIPT_PATH="${BUILD_SCRIPT_DIR}/$(basename "$0")"
BUILD_SCRIPT_SOURCE_DIR="${BUILD_SCRIPT_DIR}/../.."

BUILD_SCRIPT_CONF_PATH="${BUILD_SCRIPT_SOURCE_DIR}/config/conf.generated.bash"

# Source helper scripts
source "${BUILD_SCRIPT_SOURCE_DIR}/scripts/utils/helpers.bash"
source "${BUILD_SCRIPT_SOURCE_DIR}/scripts/utils/pretty_print.bash"
source "${BUILD_SCRIPT_SOURCE_DIR}/scripts/utils/argparse.bash"

# Parse command-line arguments
parse_args() {
  argparse_init "${BUILD_SCRIPT_PATH}" "Build AlkOS from the specified CMake configuration directory"
  argparse_add_positional "cmake_conf_dir" "Path to the CMake configuration directory" true "" "directory"
  argparse_add_option "v|verbose" "Enable verbose output" false false "" "flag"
  argparse_parse "$@"
}

# Validate the provided arguments
process_args() {
  if [ ! -d "$(argparse_get "cmake_conf_dir")" ]; then
    dump_error "The specified CMake configuration directory does not exist: $(argparse_get "cmake_conf_dir")"
    exit 1
  fi
}

# Main function
main() {
  parse_args "$@"
  process_args

  pretty_info "Building AlkOS from directory: $(argparse_get "cmake_conf_dir")"

  base_runner "Failed to build AlkOS" "$(argparse_get 'v|verbose')" cmake --build "$(argparse_get "cmake_conf_dir")" --target iso
  pretty_success "AlkOS built successfully"
}

# Run the script
main "$@"
