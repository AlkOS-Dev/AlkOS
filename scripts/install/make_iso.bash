#!/bin/bash

# Script metadata
MAKE_ISO_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
MAKE_ISO_SCRIPT_PATH="${MAKE_ISO_SCRIPT_DIR}/$(basename "$0")"

# GRUB config placeholders and default values
MAKE_ISO_SCRIPT_BOOTABLE_TOKEN="BOOTABLE_KERNEL_PLACEHOLDER"
MAKE_ISO_SCRIPT_MODULES_TOKEN="MODULES_PLACEHOLDER"
MAKE_ISO_SCRIPT_GRUB_CONTENTS="
set timeout=0
set default=0
menuentry \"AlkOS\" {
  multiboot2 /boot/${MAKE_ISO_SCRIPT_BOOTABLE_TOKEN}
  ${MAKE_ISO_SCRIPT_MODULES_TOKEN}
  boot
}
"
MAKE_ISO_SCRIPT_GRUB_PATH_IN_ISO="boot/grub/grub.cfg"
MAKE_ISO_SCRIPT_EXECUTABLE_NAME="alkos.kernel"
MAKE_ISO_SCRIPT_MODULES_LIST=""
MAKE_ISO_SCRIPT_VERBOSE=false

# Helper functions
source "${MAKE_ISO_SCRIPT_DIR}/../utils/helpers.bash"
source "${MAKE_ISO_SCRIPT_DIR}/../utils/pretty_print.bash"

help() {
  echo "${MAKE_ISO_SCRIPT_PATH} <iso_file> <sysroot> [--exec_name | -e executable_name] [--modules | -m modules] [--verbose | -v]"
  echo "Creates a .iso for alkOS from the sysroot directory"
  echo "Where:"
  echo "  iso_file  - Path to the .iso file to create (Positional, must be provided)"
  echo "  sysroot   - Path to the sysroot directory of alkOS (Positional, must be provided)"
  echo "  --exec_name | -e - Name of the executable in sysroot/boot to boot (default: alkos.kernel)"
  echo "  --modules  | -m - Space-separated list of tuples in the form module_name/module_command"
  echo "  --verbose  | -v - Enable verbose output"
}

parse_args() {
  # Ensure the first two positional arguments are provided
  if [[ $# -lt 2 ]]; then
    echo "Error: Both <target> and <source> must be provided as the first two arguments."
    help
    exit 1
  fi

  # First positional argument is the target (ISO file path)
  MAKE_ISO_SCRIPT_TARGET="$1"
  shift

  # Second positional argument is the source (sysroot directory)
  MAKE_ISO_SCRIPT_SOURCE="$1"
  shift

  # Process optional arguments
  while [[ $# -gt 0 ]]; do
    case $1 in
      -h|--help)
        help
        exit 0
        ;;
      -v|--verbose)
        MAKE_ISO_SCRIPT_VERBOSE=true
        shift
        ;;
      -e|--exec_name)
        MAKE_ISO_SCRIPT_EXECUTABLE_NAME="$2"
        shift 2
        ;;
      -m|--modules)
        MAKE_ISO_SCRIPT_MODULES_LIST="$2"
        shift 2
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
  # Validate that both target and source are set
  if [ -z "$MAKE_ISO_SCRIPT_TARGET" ] || [ -z "$MAKE_ISO_SCRIPT_SOURCE" ]; then
    dump_error "Both target and source must be provided!"
    exit 1
  fi

  # Replace the kernel executable placeholder in the GRUB contents
  MAKE_ISO_SCRIPT_GRUB_CONTENTS="${MAKE_ISO_SCRIPT_GRUB_CONTENTS//${MAKE_ISO_SCRIPT_BOOTABLE_TOKEN}/${MAKE_ISO_SCRIPT_EXECUTABLE_NAME}}"

  # Replace the modules placeholder in the GRUB contents with tuple parsing
  if [ -n "$MAKE_ISO_SCRIPT_MODULES_LIST" ]; then
    local MODULES_LINES=""
    # Iterate over each tuple provided
    for mod_tuple in $MAKE_ISO_SCRIPT_MODULES_LIST; do
      # Split the tuple into module_name and module_command based on the '/'
      module_name="${mod_tuple%%/*}"
      module_cmd="${mod_tuple##*/}"
      MODULES_LINES+="  module2 /boot/${module_name} ${module_cmd}\n"
    done
    MAKE_ISO_SCRIPT_GRUB_CONTENTS="${MAKE_ISO_SCRIPT_GRUB_CONTENTS//${MAKE_ISO_SCRIPT_MODULES_TOKEN}/$(echo -e "${MODULES_LINES}")}"
  else
    MAKE_ISO_SCRIPT_GRUB_CONTENTS="${MAKE_ISO_SCRIPT_GRUB_CONTENTS//${MAKE_ISO_SCRIPT_MODULES_TOKEN}/}"
  fi

  # Check if the GRUB contents are valid
  if [[ "$MAKE_ISO_SCRIPT_GRUB_CONTENTS" == *"${MAKE_ISO_SCRIPT_BOOTABLE_TOKEN}"* ]] ||
     [[ "$MAKE_ISO_SCRIPT_GRUB_CONTENTS" == *"${MAKE_ISO_SCRIPT_MODULES_TOKEN}"* ]]; then
    dump_error "Failed to replace tokens in the GRUB configuration!"
    exit 1
  fi

  if [ "$MAKE_ISO_SCRIPT_VERBOSE" = true ]; then
    pretty_info "Grub configuration contents:\n$MAKE_ISO_SCRIPT_GRUB_CONTENTS"
  fi
}

main() {
  parse_args "$@"
  process_args

  if [ ! -d "$MAKE_ISO_SCRIPT_SOURCE" ]; then
    dump_error "Source directory does not exist: $MAKE_ISO_SCRIPT_SOURCE"
    exit 1
  fi

  if ! command -v grub-mkrescue &> /dev/null; then
    dump_error "grub-mkrescue is not installed!"
    exit 1
  fi

  pretty_info "Creating boot directory"
  base_runner "Failed to create boot directory" "${MAKE_ISO_SCRIPT_VERBOSE}" mkdir -p "${MAKE_ISO_SCRIPT_SOURCE}/boot"

  pretty_info "Creating grub.cfg file"
  base_runner "Failed to create grub.cfg" "${MAKE_ISO_SCRIPT_VERBOSE}" \
    echo "${MAKE_ISO_SCRIPT_GRUB_CONTENTS}" > "${MAKE_ISO_SCRIPT_SOURCE}/${MAKE_ISO_SCRIPT_GRUB_PATH_IN_ISO}"

  pretty_info "Creating .iso file: $MAKE_ISO_SCRIPT_TARGET from source: $MAKE_ISO_SCRIPT_SOURCE"
  base_runner "Failed to create path to .iso file" "${MAKE_ISO_SCRIPT_VERBOSE}" mkdir -p "$(dirname "${MAKE_ISO_SCRIPT_TARGET}")"
  base_runner "Failed to create .iso file" "${MAKE_ISO_SCRIPT_VERBOSE}" \
    grub-mkrescue -o "${MAKE_ISO_SCRIPT_TARGET}" "${MAKE_ISO_SCRIPT_SOURCE}"

  pretty_success "Created .iso file: $(basename "${MAKE_ISO_SCRIPT_TARGET}")"
}

main "$@"
