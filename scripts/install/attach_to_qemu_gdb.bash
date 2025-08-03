# TODO: refactor or whatever

ATTACH_GDB_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ATTACH_GDB_SCRIPT_PATH="${ATTACH_GDB_SCRIPT_DIR}/$(basename "$0")"
ATTACH_GDB_SCRIPT_SOURCE_DIR="${ATTACH_GDB_SCRIPT_DIR}/../.."
ATTACH_GDB_ARCH="x86_64-elf"

source "${ATTACH_GDB_SCRIPT_SOURCE_DIR}/scripts/utils/helpers.bash"
source "${ATTACH_GDB_SCRIPT_SOURCE_DIR}/scripts/utils/pretty_print.bash"
source "${ATTACH_GDB_SCRIPT_SOURCE_DIR}/scripts/utils/argparse.bash"

parse_args() {
  argparse_init "${ATTACH_GDB_SCRIPT_PATH}" "Attach GDB to a running QEMU instance of AlkOS kernel"
  argparse_add_positional "kernel_source_path" "Path to the AlkOS kernel source file" true
  argparse_add_option "g|gdb" "Path to GDB executable" false "${ATTACH_GDB_SCRIPT_SOURCE_DIR}/tools/bin/${ATTACH_GDB_ARCH}-gdb" "" "string"

  argparse_parse "$@"
}

process_args() {
  if [ ! -f $(argparse_get "kernel_source_path") ]; then
    dump_error "Provided kernel source path does not exist!"
    exit 1
  fi
}

main() {
  parse_args "$@"
  process_args

  pretty_info "Attaching GDB to running kernel QEMU instance..."

  base_runner "Failed to attach to the kernel!" true "$(argparse_get "g|gdb")" "$(argparse_get "kernel_source_path")" -ex "target remote localhost:1234"
}

main "$@"
