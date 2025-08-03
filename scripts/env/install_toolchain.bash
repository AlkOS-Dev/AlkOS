#!/bin/bash

# script dirs
INSTALL_TOOLCHAIN_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

# script dirs
INSTALL_TOOLCHAIN_BUILD_SCRIPT_PATH="${INSTALL_TOOLCHAIN_DIR}/build_cross_compile.bash"
INSTALL_TOOLCHAIN_CLEAN_BUILD_SCRIPT_PATH="${INSTALL_TOOLCHAIN_DIR}/clean_build_dir.bash"

# sources
source "${INSTALL_TOOLCHAIN_DIR}/../utils/pretty_print.bash"
source "${INSTALL_TOOLCHAIN_DIR}/../utils/helpers.bash"
source "${INSTALL_TOOLCHAIN_DIR}/../utils/argparse.bash"

parse_args() {
  argparse_init "${INSTALL_TOOLCHAIN_BUILD_SCRIPT_PATH}" "Install cross-compile toolchain"
  argparse_add_positional "install_dir" "Directory to install the toolchain" true ""
  argparse_add_positional "build_dir" "Directory to store build files" true ""
  argparse_add_positional "arch" "Architecture to build the toolchain for" true "x86_64|i386"
  argparse_add_option "v|verbose" "Enable verbose output" false false "" "flag"
}

main() {
  parse_args "$@"

  # Ensure directories exists
  base_runner "Failed to create install directory" true mkdir -p "$(argparse_get "install_dir")"
  base_runner "Failed to create build directory" true mkdir -p "$(argparse_get "build_dir")"

  pretty_info "Installing cross-compile toolchain"
  for arch in ${INSTALL_TOOCHAIN_ARCH_DICT[$(argparse_get "arch")]}; do
    pretty_info "Installing for ${arch}"
    base_runner "Failed to clean build directory" true "${INSTALL_TOOLCHAIN_CLEAN_BUILD_SCRIPT_PATH}" $(argparse_get "v|verbose")
    # Spawn a subshell to run the build script (to avoid polluting the current shell)
    (
      base_runner "Failed to install cross-compile toolchain" true "${INSTALL_TOOLCHAIN_BUILD_SCRIPT_PATH}" --install \
        toolchain "$(argparse_get "v|verbose")"   \
          -t "$(argparse_get "install_dir")/${arch}" \
          -b "$(argparse_get "build_dir")/${arch}"   \
          -c "${arch}"

      base_runner "Failed to remove build directory" true rm -rf "$(argparse_get "build_dir")/${arch}"

      exit 0
    ) 2>&1 || exit 1
  done
}

main "$@"
