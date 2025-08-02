#!/bin/bash

DEBUG_REGRESSION_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
DEBUG_REGRESSION_SCRIPT="${DEBUG_REGRESSION_DIR}/$(basename "$0")"

source "${DEBUG_REGRESSION_DIR}/../../utils/helpers.bash"
source "${DEBUG_REGRESSION_DIR}/common/init.bash"
source "${DEBUG_REGRESSION_DIR}/common/parsing.bash"
source "${DEBUG_REGRESSION_DIR}/common/boot_test.bash"

main() {
  parse_args "${DEBUG_REGRESSION_SCRIPT}" "$@"

  pretty_info "Running debug regression test"

  init "debug -p regression_mode" "${ARCH}"

  # Tests
  boot_test

  pretty_success "Debug regression test passed"
}

main "$@"
