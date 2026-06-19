#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.


set -euo pipefail

CHECK_LICENSE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
source "${CHECK_LICENSE_DIR}/../utils/license_lib.bash"

main() {
  cd "${LICENSE_LIB_ROOT_DIR}"

  local rel rc checked=0 bad=0

  while IFS= read -r -d '' rel; do
    checked=$((checked + 1))
    set +e
    license_validate "${rel}"
    rc=$?
    set -e
    [[ "${rc}" -ne 0 ]] && bad=$((bad + 1))
  done < <(license_collect_files)

  echo "Checked ${checked} files."

  if ((bad > 0)); then
    echo "License header check failed: ${bad} file(s) missing or broken." >&2
    echo "Run scripts/actions/apply_license_headers.bash to add missing headers." >&2
    exit 1
  fi

  echo "All files carry a valid license header."
}

main "$@"
