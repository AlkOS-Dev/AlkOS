#!/bin/bash

FIND_FAILING_LINE_ACTION_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source "${FIND_FAILING_LINE_ACTION_DIR}/../utils/conf_handlers.bash"

source_conf_file
verify_conf_var_exists CONF_SYSROOT

FIND_FAILING_LINE_KERNEL_ELF="${CONF_SYSROOT}/boot/alkos.kernel"

if [ $# -ne 1 ]; then
    echo "Usage: $0 <RIP_HEX_ADDRESS>"
    echo "For example: $0 0xFFFFFFFF80012345"
    exit 1
fi

RIP_ADDR="$1"
STOP_ADDR=$((${RIP_ADDR} + 0x10))
STOP_ADDR_HEX=$(printf "0x%x" $STOP_ADDR)

addr2line -e "${FIND_FAILING_LINE_KERNEL_ELF}" -C "${RIP_ADDR}"
objdump -d -M intel "${FIND_FAILING_LINE_KERNEL_ELF}" \
    --start-address="${RIP_ADDR}" \
    --stop-address="${STOP_ADDR_HEX}"
