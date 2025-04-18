#!/usr/bin/env bash

CLEAN_BUILD_SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
CLEAN_BUILD_SCRIPT_PATH="${CLEAN_BUILD_SCRIPT_DIR}/$(basename "$0")"
CLEAN_BUILD_BUILD_DIR="${CLEAN_BUILD_SCRIPT_DIR}/../../build"

source "${CLEAN_BUILD_SCRIPT_DIR}/../utils/pretty_print.bash"
source "${CLEAN_BUILD_SCRIPT_DIR}/../utils/helpers.bash"

help() {
  echo "${CLEAN_BUILD_SCRIPT_PATH} [--verbose | -v]"
  echo "Where:"
  echo "--verbose | -v - flag to enable verbose output"
}

parse_args() {
  CLEAN_BUILD_VERBOSE=false
  while [[ $# -gt 0 ]]; do
    case $1 in
      -h|--help)
        help
        exit 0
        ;;
      -v|--verbose)
        CLEAN_BUILD_VERBOSE=true
        shift
        ;;
      *)
        dump_error "Unknown argument: $1"
        ;;
    esac
  done
}

process_args() {
  if [[ "${CLEAN_BUILD_VERBOSE}" == "true" ]]; then
    CLEAN_BUILD_VERBOSE_FLAG="--verbose"
  else
    CLEAN_BUILD_VERBOSE_FLAG=""
  fi
}

main() {
  parse_args "$@"
  process_args

  if [[ ! -d "${CLEAN_BUILD_BUILD_DIR}" ]]; then
    pretty_info "Build directory does not exist"
    exit 0
  fi

  base_runner "Deleting config.cache" ${CLEAN_BUILD_VERBOSE} find ${CLEAN_BUILD_BUILD_DIR} -name "config.cache" -delete
  base_runner "Finding and running distclean" ${CLEAN_BUILD_VERBOSE} \
  find "${CLEAN_BUILD_BUILD_DIR}" -type f \( -iname "Makefile" -o -iname "makefile" -o -iname "GNUmakefile" \) -printf '%h\n' | sort -u | while read -r dir; do
      (cd "$dir" && make distclean && make clean)
  done

  pretty_success "${CLEAN_BUILD_BUILD_DIR} cleaned"
}

main "$@"
