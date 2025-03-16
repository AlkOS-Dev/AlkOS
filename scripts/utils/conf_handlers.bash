#! /bin/bash

CONF_HANDLER_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
CONF_HANDLER_CONF_FILE_PATH="${CONF_HANDLER_SCRIPT_DIR}/../conf.bash"

source "${CONF_HANDLER_SCRIPT_DIR}/pretty_print.bash"

verify_conf_file_exists() {
  if [ ! -f "${CONF_HANDLER_CONF_FILE_PATH}" ] ; then
    pretty_error "Configuration file not found at ${CONF_HANDLER_CONF_FILE_PATH}"
    exit 1
  fi
}

is_conf_file_present() {
  if [ -f "${CONF_HANDLER_CONF_FILE_PATH}" ] ; then
    return 0
  else
    return 1
  fi
}

verify_conf_var_exists() {
  local var_name="$1"

  if [ -z "${!var_name}" ]; then
    pretty_error "Configuration variable ${var_name} not found in ${CONF_HANDLER_CONF_FILE_PATH}"
    exit 1
  fi
}

source_conf_file() {
  verify_conf_file_exists

  source "${CONF_HANDLER_CONF_FILE_PATH}"
}
