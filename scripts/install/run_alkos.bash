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

# Flags
RUN_ALKOS_SCRIPT_VERBOSE=false
RUN_ALKOS_SCRIPT_GDB=false

# Sources
source "${RUN_ALKOS_SCRIPT_SOURCE_DIR}/scripts/utils/helpers.bash"
source "${RUN_ALKOS_SCRIPT_SOURCE_DIR}/scripts/utils/pretty_print.bash"

help() {
  echo "${RUN_ALKOS_SCRIPT_PATH} <alkos_iso_path> [--verbose | -v] [--gdb | -g]"
  echo "Where:"
  echo "alkos_iso_path - path to the .iso file (Positional, must be provided)"
  echo "--verbose | -v - flag to enable verbose output"
  echo "--gdb     | -g - flag to run AlkOS in QEMU with GDB"
}

parse_args() {
  # Ensure the first positional argument (ISO file path) is provided
  if [[ $# -lt 1 ]]; then
    echo "Error: No path to the .iso file was provided!"
    help
    exit 1
  fi

  # First positional argument is the path to the .iso file
  RUN_ALKOS_ISO_PATH="$1"
  shift

  # Process optional flags
  while [[ $# -gt 0 ]]; do
    case $1 in
      -h|--help)
        help
        exit 0
        ;;
      -v|--verbose)
        RUN_ALKOS_SCRIPT_VERBOSE=true
        shift
        ;;
      -g|--gdb)
        RUN_ALKOS_SCRIPT_GDB=true
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

process_args() {
  # Ensure that the .iso file path is provided
  if [ -z "$RUN_ALKOS_ISO_PATH" ]; then
    dump_error "No path to the .iso file was provided!"
    exit 1
  fi

  # Update QEMU arguments to include the ISO path
  RUN_ALKOS_SCRIPT_QEMU_ARGS="${RUN_ALKOS_SCRIPT_QEMU_ARGS} -cdrom ${RUN_ALKOS_ISO_PATH}"

  # If GDB flag is set, add GDB arguments to QEMU command
  if [ "$RUN_ALKOS_SCRIPT_GDB" = true ] ; then
    RUN_ALKOS_SCRIPT_QEMU_ARGS="${RUN_ALKOS_SCRIPT_QEMU_ARGS} ${RUN_ALKOS_SCRIPT_GDB_ARGS}"
  fi
}

main() {
  parse_args "$@"
  process_args

  pretty_info "Running AlkOS in QEMU"
  # shellcheck disable=SC2086
  # RUN_ALKOS_SCRIPT_QEMU_ARGS is intentionally unquoted
  base_runner "Failed to run AlkOS in QEMU" "${RUN_ALKOS_SCRIPT_VERBOSE}" "${RUN_ALKOS_SCRIPT_QEMU_COMMAND}" ${RUN_ALKOS_SCRIPT_QEMU_ARGS}
}

main "$@"
