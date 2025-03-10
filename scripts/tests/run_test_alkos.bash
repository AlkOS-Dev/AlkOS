#!/bin/bash

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source "${SCRIPT_DIR}/../utils/conf_handlers.bash"
source_conf_file
verify_conf_var_exists CONF_QEMU_COMMAND
verify_conf_var_exists CONF_QEMU_TEST_FLAGS

RUN_TEST_ALKOS_EXEC_LINE="${CONF_QEMU_COMMAND} ${CONF_QEMU_TEST_FLAGS} -cdrom ${CONF_ISO_PATH}"
echo "Running test Alkos with command: ${RUN_TEST_ALKOS_EXEC_LINE}"
eval "${RUN_TEST_ALKOS_EXEC_LINE}"