#!/bin/bash

CHECK_CLANG_FORMAT_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
CHECK_CLANG_FORMAT_FORMAT_FILE="${CHECK_CLANG_FORMAT_SCRIPT_DIR}/../../.clang-format"
CHECK_CLANG_FORMAT_UTILS_DIR="${CHECK_CLANG_FORMAT_SCRIPT_DIR}/../utils/clang_format_utils.bash"

source $CHECK_CLANG_FORMAT_UTILS_DIR

apply_clang_format() {
  local files=("$@")

  echo "Applying clang-format using $(nproc) CPU cores..."

  parallel -j $(nproc) clang-format -style="file:${CHECK_CLANG_FORMAT_FORMAT_FILE}" -i ::: "${files[@]}"

  echo "Clang-format applied to all files."
}

main() {
  echo "Running clang-format..."

  files_to_format=$(find_files)

  if [ -z "$files_to_format" ]; then
    echo "No source files found to format."
    exit 0
  fi

  apply_clang_format $files_to_format
}

main
