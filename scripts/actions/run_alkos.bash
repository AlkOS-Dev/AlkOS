#!/bin/bash

RUN_ALKOS_ACTION_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source "${RUN_ALKOS_ACTION_DIR}/../utils/conf_handlers.bash"

CONF_FILE_OVERRIDE=""
PASSTHROUGH_ARGS=()

# Parse arguments to find --conf and other potential flags
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
verify_conf_var_exists CONF_ISO_PATH
"${RUN_ALKOS_ACTION_DIR}/../install/run_alkos.bash" "${CONF_ISO_PATH}" "${PASSTHROUGH_ARGS[@]}"
