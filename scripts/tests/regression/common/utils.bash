#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

REGRESSION_UTILS_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source "${REGRESSION_UTILS_DIR}/../../../utils/helpers.bash"
source "${REGRESSION_UTILS_DIR}/../../../utils/pretty_print.bash"

verify_output_contains() {
  local output="$1"
  local expected="$2"

  if [[ ! "${output}" == *"${expected}"* ]]; then
    pretty_error "Output does not contain expected string"
    pretty_error "Expected: ${expected}"
    exit 1
  fi
}

verify_output_not_contains() {
  local output="$1"
  local expected="$2"

  if [[ "${output}" == *"${expected}"* ]]; then
    pretty_error "Output contains unexpected string"
    pretty_error "Unexpected: ${expected}"
    exit 1
  fi
}
