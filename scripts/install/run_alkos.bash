#!/bin/bash

# Directories variables
RUN_ALKOS_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
RUN_ALKOS_SCRIPT_PATH="${RUN_ALKOS_SCRIPT_DIR}/$(basename "$0")"
RUN_ALKOS_SCRIPT_SOURCE_DIR="${RUN_ALKOS_SCRIPT_DIR}/../.."

source "${RUN_ALKOS_SCRIPT_DIR}/../utils/conf_handlers.bash"

source_conf_file
verify_conf_var_exists CONF_QEMU_COMMAND
verify_conf_var_exists CONF_QEMU_NORMAL_FLAGS

# qemu command
RUN_ALKOS_SCRIPT_QEMU_COMMAND="${CONF_QEMU_COMMAND}"
RUN_ALKOS_SCRIPT_GDB_ARGS="-s -S"
RUN_ALKOS_SCRIPT_QEMU_ARGS="${CONF_QEMU_NORMAL_FLAGS}"

# Sources
source "${RUN_ALKOS_SCRIPT_SOURCE_DIR}/scripts/utils/helpers.bash"
source "${RUN_ALKOS_SCRIPT_SOURCE_DIR}/scripts/utils/pretty_print.bash"
source "${RUN_ALKOS_SCRIPT_DIR}/../utils/argparse.bash"

parse_args() {
  argparse_init "${RUN_ALKOS_SCRIPT_PATH}" "Run AlkOS in QEMU with an ISO file"
  argparse_add_positional "alkos_iso_path" "Path to the AlkOS ISO file" true
  argparse_add_option "v|verbose" "Enable verbose output" false false "" "flag"
  argparse_add_option "g|gdb" "Run AlkOS in QEMU with GDB" false false "" "flag"

  argparse_parse "$@"
}

process_args() {
  # Update QEMU arguments to include the ISO path
  RUN_ALKOS_SCRIPT_QEMU_ARGS="${RUN_ALKOS_SCRIPT_QEMU_ARGS} -cdrom $(argparse_get 'alkos_iso_path')"

  # If GDB flag is set, add GDB arguments to QEMU command
  if [[ $(argparse_get 'g|gdb') == true ]] ; then
    RUN_ALKOS_SCRIPT_QEMU_ARGS="${RUN_ALKOS_SCRIPT_QEMU_ARGS} ${RUN_ALKOS_SCRIPT_GDB_ARGS}"
  fi
}

main() {
  parse_args "$@"
  process_args

  pretty_info "Running AlkOS in QEMU"
  # shellcheck disable=SC2086
  # RUN_ALKOS_SCRIPT_QEMU_ARGS is intentionally unquoted
  base_runner "Failed to run AlkOS in QEMU" $(argparse_get "v|verbose") "${RUN_ALKOS_SCRIPT_QEMU_COMMAND}" ${RUN_ALKOS_SCRIPT_QEMU_ARGS}
}

main "$@"
