#!/bin/bash

BOOT_TEST_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source "${BOOT_TEST_DIR}/../../../utils/pretty_print.bash"
source "${BOOT_TEST_DIR}/utils.bash"

boot_test() {
    pretty_info "Running boot test"

    local output_file="/tmp/alkOS_regression.log"
    local test_pid

    "${BOOT_TEST_DIR}/../../run_test_alkos.bash" > "$output_file" 2>&1 &
    test_pid=$!
    sleep 15

    # Kill the test process if it's still running
    if kill -0 $test_pid 2>/dev/null; then
        kill $test_pid
    fi

    local output=$(<"$output_file")
    verify_output_contains "$output" "Hello from AlkOS!"
    verify_output_not_contains "$output" "ERROR"
    verify_output_not_contains "$output" "WARNING"
}