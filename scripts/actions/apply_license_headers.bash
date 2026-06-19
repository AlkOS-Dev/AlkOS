#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

set -euo pipefail

APPLY_LICENSE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
source "${APPLY_LICENSE_DIR}/../utils/license_lib.bash"
source "${APPLY_LICENSE_DIR}/../utils/pretty_print.bash"

main() {
  cd "${LICENSE_LIB_ROOT_DIR}"

  local added=0 skipped=0
  local -a broken=()
  local rel rc

  while IFS= read -r -d '' rel; do
    set +e
    license_apply "${rel}"
    rc=$?
    set -e
    case "${rc}" in
      0)
        added=$((added + 1))
        pretty_info "added header: ${rel}"
        ;;
      3 | 2) skipped=$((skipped + 1)) ;;
      *) broken+=("${rel}") ;;
    esac
  done < <(license_collect_files)

  pretty_success "Headers added: ${added}, already present: ${skipped}"

  if ((${#broken[@]} > 0)); then
    pretty_error "Broken headers (fix manually):"
    printf '  %s\n' "${broken[@]}" >&2
    exit 1
  fi
}

main "$@"
