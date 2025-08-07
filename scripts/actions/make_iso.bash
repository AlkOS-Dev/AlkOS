#!/bin/bash

MAKE_ISO_ACTION_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source "${MAKE_ISO_ACTION_DIR}/../utils/conf_handlers.bash"

CONF_FILE_OVERRIDE=""
PASSTHROUGH_ARGS=()

# Parse arguments to find --conf
while [[ $# -gt 0 ]]; do
  case $1 in
    --conf)
      CONF_FILE_OVERRIDE="$2"
      shift 2
      ;;
    *)
      PASSTHROUGH_ARGS+=("$1")
      shift
      ;;
  esac
done

# If a conf file is provided, override the default path
if [[ -n "$CONF_FILE_OVERRIDE" ]]; then
  CONF_HANDLER_CONF_FILE_PATH="$CONF_FILE_OVERRIDE"
fi

source_conf_file
verify_conf_var_exists CONF_SYSROOT
verify_conf_var_exists CONF_ISO_PATH
verify_conf_var_exists CONF_KERNEL_MODULES
verify_conf_var_exists CONF_BOOTABLE_KERNEL_EXEC
verify_conf_var_exists CONF_KERNEL_COMMANDS

# Split the CONF_KERNEL_MODULES and CONF_KERNEL_COMMANDS into arrays
IFS=' ' read -r -a modules <<< "$CONF_KERNEL_MODULES"
IFS=' ' read -r -a commands <<< "$CONF_KERNEL_COMMANDS"

# Check if both arrays have the same number of elements
if [ ${#modules[@]} -ne ${#commands[@]} ]; then
  echo "Error: The number of kernel modules and commands do not match."
  exit 1
fi

# Combine the arrays into a single tuple string "module/command"
tuple_str=""
for i in "${!modules[@]}"; do
  tuple_str+="${modules[$i]}/${commands[$i]} "
done
# Trim any trailing whitespace
tuple_str=$(echo "$tuple_str" | sed 's/[[:space:]]*$//')

# Call the make_iso script with the tuple string for modules
"${MAKE_ISO_ACTION_DIR}/../install/make_iso.bash" "${CONF_ISO_PATH}" "${CONF_SYSROOT}" -e "${CONF_BOOTABLE_KERNEL_EXEC}" -m "${tuple_str}" "${PASSTHROUGH_ARGS[@]}"
