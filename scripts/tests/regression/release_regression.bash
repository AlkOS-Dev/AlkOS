#!/bin/bash

RELEASE_REGRESSION_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
RELEASE_REGRESSION_SCRIPT="${RELEASE_REGRESSION_DIR}/$(basename "$0")"

source "${RELEASE_REGRESSION_DIR}/../../utils/helpers.bash"
source "${RELEASE_REGRESSION_DIR}/common/init.bash"
source "${RELEASE_REGRESSION_DIR}/common/parsing.bash"
source "${RELEASE_REGRESSION_DIR}/common/boot_test.bash"

main () {
  parse_args "${RELEASE_REGRESSION_SCRIPT}" "$@"

  pretty_info "Running release regression test"

  init "release -p regression_mode" "${ARCH}"

  # Tests
  boot_test

  pretty_success "Release regression test passed"
}

main "$@"
