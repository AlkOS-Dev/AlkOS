# TODO: refactor or whatever

ATTACH_GDB_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ATTACH_GDB_SCRIPT_PATH="${ATTACH_GDB_SCRIPT_DIR}/$(basename "$0")"
ATTACH_GDB_SCRIPT_SOURCE_DIR="${ATTACH_GDB_SCRIPT_DIR}/../.."
ATTACH_GDB_ARCH="x86_64-elf"

source "${ATTACH_GDB_SCRIPT_SOURCE_DIR}/scripts/utils/helpers.bash"
source "${ATTACH_GDB_SCRIPT_SOURCE_DIR}/scripts/utils/pretty_print.bash"

help() {
  echo "${ATTACH_GDB_SCRIPT_PATH} [alkos.kernel - binary file path] [-g | --gdb GDB_PATH] "
  echo "Where:"
  echo "alkos.kernel - path to the AlkOS kernel binary file"
  echo "--gdb     | -g - flag to provide custom path to GDB"
}

ATTACH_GDB_GDB_PATH=""
ATTACH_GDB_KERNEL_SOURCE_PATH=""

parse_args() {

  while [[ $# -gt 0 ]]; do
    case $1 in
      -h|--help)
        help
        exit 0
        ;;
      -g|--gdb)
        shift
        ATTACH_GDB_GDB_PATH="$1"
        shift
        ;;
      *)
        if [ -z "$ATTACH_GDB_KERNEL_SOURCE_PATH" ]; then
          ATTACH_GDB_KERNEL_SOURCE_PATH="$1"
        else
          echo "Unknown argument: $1"
          exit 1
        fi
        shift
        ;;
    esac
  done
}

process_args() {
  if [ -z "$ATTACH_GDB_KERNEL_SOURCE_PATH" ]; then
    dump_error "No path to the kernel sources provided!"
    exit 1
  fi

  if [ ! -f "$ATTACH_GDB_KERNEL_SOURCE_PATH" ]; then
    dump_error "Provided kernel source path does not exist!"
    exit 1
  fi

  if [ -z "$ATTACH_GDB_GDB_PATH" ]; then
    ATTACH_GDB_GDB_PATH="${ATTACH_GDB_SCRIPT_SOURCE_DIR}/tools/bin/${ATTACH_GDB_ARCH}-gdb"
    pretty_info "Using default GDB path: ${ATTACH_GDB_GDB_PATH}"
  fi
}

main() {
  parse_args "$@"
  process_args

  pretty_info "Attaching GDB to running kernel QEMU instance..."

  base_runner "Failed to attach to the kernel!" true "${ATTACH_GDB_GDB_PATH}" "${ATTACH_GDB_KERNEL_SOURCE_PATH}" -ex "target remote localhost:1234"
}

main "$@"
