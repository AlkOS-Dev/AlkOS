#!/bin/bash

HELPERS_SCRIPT_DIR="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
HELPERS_LOG_FILE="/tmp/alkOS_build.log"

source "${HELPERS_SCRIPT_DIR}/pretty_print.bash"

dump_error() {
  help

  pretty_error "$1"
  exit 1
}

assert_always() {
  dump_error "ASSERT failed in ${BASH_SOURCE[1]} at line ${BASH_LINENO[0]}"
}

assert_argument_provided() {
  if [ -z "$1" ]; then
    dump_error "ASSERT: argument was not provided, failed in ${BASH_SOURCE[1]} at line ${BASH_LINENO[0]}"
  fi
}

base_runner() {
    local dump_info="$1"
    shift
    local verbose="$1"
    shift

    if [ "$verbose" = true ]; then
        if ! "$@" 2>&1 | tee "$HELPERS_LOG_FILE"; then
            dump_error "${dump_info}"
        fi
    else
        if ! "$@" > "$HELPERS_LOG_FILE" 2>&1; then
            dump_error "${dump_info}"
        fi
    fi
}

check_is_in_env_path() {
    assert_argument_provided "$1"

    local new_path="$1"

    pretty_info "Checking if ${new_path} is in PATH"

    IFS=':' read -ra path_array <<< "${PATH}"

    local existing_path
    for existing_path in "${path_array[@]}"; do
        if [ "$existing_path" = "${new_path}" ]; then
            pretty_info "Path: ${new_path} already exists in PATH"
            return 0
        fi
    done

    pretty_info "Path: ${new_path} does not exist in PATH"
    return 1
}

add_to_user_env_path() {
  assert_argument_provided "$1"
  assert_argument_provided "$2"

  local new_path="$1"
  local is_added="$2"
  pretty_info "Adding ${new_path} to PATH"

  if [ "${is_added}" = false ]; then
    echo "export PATH=\"${new_path}:\$PATH\"" >> ~/.bashrc
    echo "export PATH=\"${new_path}:\$PATH\"" >> ~/.profile
    pretty_success "Path: ${new_path} added to PATH"
  else
    pretty_info "Path: ${new_path} already exists in PATH"
  fi
}
