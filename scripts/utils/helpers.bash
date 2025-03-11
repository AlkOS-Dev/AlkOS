#!/bin/bash

HELPERS_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
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
    assert_argument_provided "$1"
    local dump_info="$1"
    shift
    assert_argument_provided "$1"
    local verbose="$1"
    shift

    pretty_info "Running: $@" >&2

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

attempt_runner() {
    assert_argument_provided "$1"
    local dump_info="$1"
    shift

    pretty_info "Running (attempt_runner): $@"

    local ret
    if [ "${CROSS_COMPILE_BUILD_VERBOSE}" = true ]; then
        "$@" 2>&1 | tee "${HELPERS_LOG_FILE}"
        ret=${PIPESTATUS[0]}
    else
        "$@" > "${HELPERS_LOG_FILE}" 2>&1
        ret=$?
    fi

    if [ ${ret} -ne 0 ]; then
        pretty_error "${dump_info}"
    fi

    return ${ret}
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

get_supported_distro_name() {
    cat /etc/*-release | tr [:upper:] [:lower:] | grep -Poi '(arch|ubuntu)' | uniq
}
