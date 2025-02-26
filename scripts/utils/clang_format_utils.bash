#!/bin/bash

declare -a CLANG_FORMAT_UTILS_DIRS=(
    "${CHECK_CLANG_FORMAT_SCRIPT_DIR}/../../alkos/kernel"
    "${CHECK_CLANG_FORMAT_SCRIPT_DIR}/../../alkos/libc"
)

find_files() {
  local files=()
  for dir in "${CLANG_FORMAT_UTILS_DIRS[@]}"; do
    files+=($(find "$dir" -type f \( -iname "*.cpp" -o -iname "*.c" -o -iname "*.h" -o -iname "*.hpp" \)))
  done
  echo "${files[@]}"
}
