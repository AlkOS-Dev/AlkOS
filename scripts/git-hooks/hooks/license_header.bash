#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

set -e

LICENSE_HEADER_HOOK_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
source "${LICENSE_HEADER_HOOK_DIR}/../helpers.bash"
source "${LICENSE_HEADER_HOOK_DIR}/../../utils/license_lib.bash"

# ==========================
# Main Script Execution
# ==========================

main() {
  local -a changed=()
  local file rc

  for file in "$@"; do
    license_is_excluded "${file}" && continue
    license_comment_style "${file}" &>/dev/null || continue
    [[ -f "${file}" ]] || continue

    set +e
    license_apply "${file}"
    rc=$?
    set -e

    case "${rc}" in
      0)
        changed+=("${file}")
        log "Added license header: ${file}"
        ;;
      2 | 3) : ;;
      *)
        log "License header broken (fix manually): ${file}"
        exit 1
        ;;
    esac
  done

  if ((${#changed[@]} == 0)); then
    log "No license headers to add"
    exit 0
  fi

  printf '%s\0' "${changed[@]}" | xargs -0 git add || {
    log "Failed to re-add files to the staging area"
    exit 1
  }
}

main "$@"
