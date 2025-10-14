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

# Helper functions
source "${MAKE_ISO_SCRIPT_DIR}/../utils/helpers.bash"
source "${MAKE_ISO_SCRIPT_DIR}/../utils/pretty_print.bash"
source "${MAKE_ISO_SCRIPT_DIR}/../utils/argparse.bash"

parse_args() {
  argparse_init "${MAKE_ISO_SCRIPT_PATH}" "Create a bootable AlkOS ISO from a sysroot directory"
  argparse_add_positional "iso_file" "Path to the .iso file to create" true "" "file"
  argparse_add_positional "sysroot" "Path to the sysroot directory of AlkOS" true "" "directory"
  argparse_add_option "e|exec_name" "Name of the executable in sysroot/boot to boot" false "alkos.kernel" "" "string"
  argparse_add_option "m|modules" "Space-separated list of tuples in the form module_name/module_command" false "" "" "list" " "
  argparse_add_option "v|verbose" "Enable verbose output" false false "" "flag"

  argparse_parse "$@"
}

process_args() {
  # Replace the kernel executable placeholder in the GRUB contents
  MAKE_ISO_SCRIPT_GRUB_CONTENTS="${MAKE_ISO_SCRIPT_GRUB_CONTENTS//${MAKE_ISO_SCRIPT_BOOTABLE_TOKEN}/$(argparse_get "e|exec_name")}"

  # Replace the modules placeholder in the GRUB contents with tuple parsing
  if [ -n "$(argparse_get "m|modules")" ]; then
    local MODULES_LINES=""
    # Iterate over each tuple provided
    for mod_tuple in $(argparse_get "m|modules"); do
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

  if [ "$(argparse_get "v|verbose")" = true ]; then
    pretty_info "Grub configuration contents:\n$MAKE_ISO_SCRIPT_GRUB_CONTENTS"
  fi
}

main() {
  parse_args "$@"
  process_args

  if [ ! -d "$(argparse_get "sysroot")" ]; then
    dump_error "Source directory does not exist: $(argparse_get "sysroot")"
    exit 1
  fi

  if ! command -v grub-mkrescue &> /dev/null; then
    dump_error "grub-mkrescue is not installed!"
    exit 1
  fi

  pretty_info "Creating boot directory"
  base_runner "Failed to create boot directory" "$(argparse_get "v|verbose")" mkdir -p "$(argparse_get "sysroot")/boot"

  pretty_info "Creating grub.cfg file"
  base_runner "Failed to create grub.cfg" "$(argparse_get "v|verbose")" \
    echo "${MAKE_ISO_SCRIPT_GRUB_CONTENTS}" > "$(argparse_get "sysroot")/${MAKE_ISO_SCRIPT_GRUB_PATH_IN_ISO}"

  pretty_info "Creating .iso file: $(argparse_get "iso_file") from source: $(argparse_get "sysroot")"
  base_runner "Failed to create path to .iso file" "$(argparse_get "v|verbose")" mkdir -p "$(dirname "$(argparse_get "iso_file")")"
  base_runner "Failed to create .iso file" "$(argparse_get "v|verbose")" \
    grub-mkrescue -o "$(argparse_get "iso_file")" "$(argparse_get "sysroot")"

  pretty_success "Created .iso file: $(basename "$(argparse_get "iso_file")")"
}

main "$@"
