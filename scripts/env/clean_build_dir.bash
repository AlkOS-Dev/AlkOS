#!/usr/bin/env bash

CLEAN_BUILD_SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
CLEAN_BUILD_SCRIPT_PATH="${CLEAN_BUILD_SCRIPT_DIR}/$(basename "$0")"
CONF_BUILD_DIR="${CLEAN_BUILD_SCRIPT_DIR}/../../build"

source "${CLEAN_BUILD_SCRIPT_DIR}/../utils/conf_handlers.bash"

source_conf_file
verify_conf_var_exists CONF_BUILD_DIR

source "${CLEAN_BUILD_SCRIPT_DIR}/../utils/pretty_print.bash"
source "${CLEAN_BUILD_SCRIPT_DIR}/../utils/helpers.bash"
source "${CLEAN_BUILD_SCRIPT_DIR}/../utils/argparse.bash"

parse_args() {
  argparse_init "${CLEAN_BUILD_SCRIPT_PATH}" "Clean build directory"
  argparse_add_option "v|verbose" "Enable verbose output" false false "" "flag"

  argparse_parse "$@"
}

main() {
  parse_args "$@"

  if [[ ! -d "${CONF_BUILD_DIR}" ]]; then
    pretty_info "Build directory does not exist"
    exit 0
  fi

  base_runner "Deleting config.cache" $(argparse_get "v|verbose") find ${CONF_BUILD_DIR} -name "config.cache" -delete
  base_runner "Finding and running distclean" $(argparse_get "v|verbose") \
  find "${CONF_BUILD_DIR}" -type f \( -iname "Makefile" -o -iname "makefile" -o -iname "GNUmakefile" \) -printf '%h\n' | sort -u | while read -r dir; do
      (cd "$dir" && make distclean && make clean)
  done

  pretty_success "${CONF_BUILD_DIR} cleaned"
}

main "$@"
