#!/bin/bash

set -euo pipefail  # Exit on error, unset variables, and pipeline failures

BUILD_AND_RUN_TESTS_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
BUILD_AND_RUN_TESTS_ARCH="x86_64"

if [ -n "${1-}" ]; then
    BUILD_AND_RUN_TESTS_ARCH="$1"
fi

# Step 1: Configure the environment for tests
"${BUILD_AND_RUN_TESTS_DIR}/../config/configure.bash" "${BUILD_AND_RUN_TESTS_ARCH}" debug_qemu_tests -v

# Step 2: Build the project
"${BUILD_AND_RUN_TESTS_DIR}/../actions/build_alkos.bash" -v

# Step 3: Run tests
shift
"${BUILD_AND_RUN_TESTS_DIR}/../actions/run_tests.bash" "$@"
