#!/bin/bash

PARSE_BASH_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source "${PARSE_BASH_DIR}/../../../utils/pretty_print.bash"
source "${PARSE_BASH_DIR}/../../../utils/argparse.bash"

export VERBOSE_FLAG=""

parse_args() {
    local script_name="$1"
    shift
    local script_description="$1"
    shift

    argparse_quick_setup "${script_name}" "${script_description}"
    argparse_add_positional 'architecture' 'Target architecture' true 'x86_64'
    argparse_parse "$@"

    ARCH=$(argparse_get 'architecture')

    if [[ $(argparse_get "v|verbose") == true ]]; then
        VERBOSE_FLAG="--verbose"
    fi
}
