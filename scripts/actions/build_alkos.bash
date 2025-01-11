#!/bin/bash

BUILD_ALKOS_ACTION_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source "${BUILD_ALKOS_ACTION_DIR}/../utils/conf_handlers.bash"

source_conf_file
verify_conf_var_exists CONF_BUILD_DIR
"${BUILD_ALKOS_ACTION_DIR}/../install/build_alkos.bash" "$CONF_BUILD_DIR"/alkos "$@"
