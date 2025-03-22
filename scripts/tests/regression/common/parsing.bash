#!/bin/bash

PARSE_BASH_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source "${PARSE_BASH_DIR}/../../../utils/pretty_print.bash"

export VERBOSE_FLAG=""

PARSING_RUN_PROVIDED=""

help() {
    local script_name="$1"

    cat << EOF
Usage: ${script_name} <architecture> [options]

Options:
    -r, --run                   Expected to run the script
    -v, --verbose               Enable verbose output
    -h, --help                  Display this help message

Examples:
    ${script_name} x86_64 --verbose

EOF
}

parse_args() {
    local script_name="$1"
    shift

    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                help "${script_name}"
                exit 0
                ;;
            -r|--run)
                PARSING_RUN_PROVIDED=true
                shift
                ;;
            -v|--verbose)
                VERBOSE_FLAG="-v --verbose"
                shift
                ;;
            *)
                if [[ -z "$ARCH" ]]; then
                  ARCH="$1"
                else
                  pretty_error "Unknown argument: $1"
                  exit 1
                fi
                shift
                ;;
        esac
    done

    if [[ -z "$ARCH" ]]; then
        pretty_error "No architecture specified"
        help "${script_name}"
        exit 1
    fi

    if [[ -z "$PARSING_RUN_PROVIDED" ]]; then
        pretty_error "Run flag must be provided to run the script!"
        help "${script_name}"
        exit 1
    fi
}
