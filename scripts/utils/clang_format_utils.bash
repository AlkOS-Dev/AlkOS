#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

declare -a CLANG_FORMAT_UTILS_DIRS=(
    "${CHECK_CLANG_FORMAT_SCRIPT_DIR}/../../alkos/kernel"
    "${CHECK_CLANG_FORMAT_SCRIPT_DIR}/../../alkos/libc"
)

find_files() {
  local files=()
  for dir in "${CLANG_FORMAT_UTILS_DIRS[@]}"; do
    files+=($(find "$dir" -type f \( -iname "*.cpp" -o -iname "*.c" -o -iname "*.h" -o -iname "*.hpp" -o -iname "*.tpp" \)))
  done
  echo "${files[@]}"
}
