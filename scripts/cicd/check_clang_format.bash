#!/bin/bash

CHECK_CLANG_FORMAT_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
CHECK_CLANG_FORMAT_FORMAT_FILE="${CHECK_CLANG_FORMAT_SCRIPT_DIR}/../../.clang-format"
CHECK_CLANG_FORMAT_UTILS_DIR="${CHECK_CLANG_FORMAT_SCRIPT_DIR}/../utils/clang_format_utils.bash"

source "$CHECK_CLANG_FORMAT_UTILS_DIR"

check_clang_format() {
  local files=("$@")

  echo "Checking clang-format using $(nproc) CPU cores..."

  parallel -j "$(nproc)" --halt now,fail=1 clang-format -style="file:${CHECK_CLANG_FORMAT_FORMAT_FILE}" --dry-run -Werror ::: "${files[@]}"

  if [ $? -ne 0 ]; then
    echo "Some files are not correctly clang-formatted."
    exit 1
  else
    echo "All files are correctly clang-formatted."
  fi
}

main() {
  echo "Checking clang-format..."

  # NOTE: Omitting osl.cpp as it generate strange errors with clang-format on github actions
  files_to_check=$(find_files | tr ' ' '\n' | grep -v '/osl\.cpp$')

  if [ -z "$files_to_check" ]; then
    echo "No source files found to check."
    exit 0
  fi

  check_clang_format $files_to_check
}

main
