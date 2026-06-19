#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

LICENSE_LIB_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
LICENSE_LIB_ROOT_DIR="${LICENSE_LIB_DIR}/../.."

LICENSE_LIB_SPDX_ID="MIT"
LICENSE_LIB_COPYRIGHT_YEARS="2025-2026"
LICENSE_LIB_COPYRIGHT_HOLDER="The AlkOS Authors"
LICENSE_LIB_AUTHORS_NOTE="See the AUTHORS file for the full list of contributors."

LICENSE_LIB_EXCLUDES=(
  "userspace/programs/doom/*"
  "rootfs/overlay/doom/*"
  "kernel/thirdparty/*"
  "tools/*"
  "build/*"
  "generated/include/autogen/*"
  "generated/src/*"
  "scripts/completions/output/*"
  "*/__pycache__/*"
)

_license_lib_err() {
  echo "license_lib: $*" >&2
}

license_comment_style() {
  local path="$1"
  local base="${path##*/}"
  local ext="${path##*.}"

  case "${base}" in
    CMakeLists.txt)
      printf '#\t\n'
      return 0
      ;;
  esac

  case "${ext,,}" in
    c | h | cc | cxx | cpp | hpp | hxx | tpp | cuh | cu | inl) printf '//\t\n' ;;
    bash | sh | cmake | py) printf '#\t\n' ;;
    nasm | asm) printf ';\t\n' ;;
    ld | lds) printf '/*\t */\n' ;;
    *) return 1 ;;
  esac
}

license_is_excluded() {
  local rel="$1" glob
  for glob in "${LICENSE_LIB_EXCLUDES[@]}"; do
    [[ "${rel}" == ${glob} ]] && return 0
  done
  return 1
}

_license_lib_content_lines() {
  printf '%s\n' "SPDX-License-Identifier: ${LICENSE_LIB_SPDX_ID}"
  printf '%s\n' "Copyright (c) ${LICENSE_LIB_COPYRIGHT_YEARS} ${LICENSE_LIB_COPYRIGHT_HOLDER}"
  printf '%s\n' "${LICENSE_LIB_AUTHORS_NOTE}"
}

license_render_header() {
  local style prefix suffix line
  style="$(license_comment_style "$1")" || return 1
  IFS=$'\t' read -r prefix suffix <<< "${style}"

  while IFS= read -r line; do
    printf '%s %s%s\n' "${prefix}" "${line}" "${suffix}"
  done < <(_license_lib_content_lines)
}

_license_lib_patterns() {
  printf '%s\n' "^SPDX-License-Identifier: ${LICENSE_LIB_SPDX_ID}\$"
  printf '%s\n' "^Copyright \\(c\\) [0-9]{4}(-[0-9]{4})? ${LICENSE_LIB_COPYRIGHT_HOLDER}\$"
  printf '%s\n' "^See the AUTHORS file for the full list of contributors\\.\$"
}

license_validate() {
  local path="$1"
  local style prefix suffix
  style="$(license_comment_style "${path}")" || return 2
  IFS=$'\t' read -r prefix suffix <<< "${style}"

  if [[ ! -f "${path}" ]]; then
    _license_lib_err "${path}: not a regular file"
    return 1
  fi

  local spdx_anchor="${prefix} SPDX-License-Identifier: ${LICENSE_LIB_SPDX_ID}${suffix}"
  local spdx_count
  spdx_count="$(grep -F -x -c -- "${spdx_anchor}" "${path}" || true)"

  if [[ "${spdx_count}" -eq 0 ]]; then
    _license_lib_err "${path}: missing license header"
    return 1
  fi
  if [[ "${spdx_count}" -gt 1 ]]; then
    _license_lib_err "${path}: duplicate license header (${spdx_count} SPDX lines)"
    return 1
  fi

  local -a patterns=()
  local p
  while IFS= read -r p; do patterns+=("${p}"); done < <(_license_lib_patterns)

  local -a head=()
  local line max=$((${#patterns[@]} + 2))
  while IFS= read -r line && ((${#head[@]} < max)); do
    head+=("${line}")
  done < "${path}"

  local start=0
  [[ "${head[0]:-}" == '#!'* ]] && start=1

  local i actual stripped
  for i in "${!patterns[@]}"; do
    actual="${head[$((start + i))]:-}"
    if [[ "${actual}" != "${prefix} "* ]] || [[ "${actual}" != *"${suffix}" ]]; then
      _license_lib_err "${path}: header broken or not at top of file (line $((start + i + 1)))"
      return 1
    fi
    stripped="${actual#"${prefix} "}"
    stripped="${stripped%"${suffix}"}"
    if [[ ! "${stripped}" =~ ${patterns[$i]} ]]; then
      _license_lib_err "${path}: header line $((start + i + 1)) does not match expected text"
      return 1
    fi
  done

  return 0
}

license_apply() {
  local path="$1"
  local style prefix suffix
  style="$(license_comment_style "${path}")" || return 2
  IFS=$'\t' read -r prefix suffix <<< "${style}"

  license_validate "${path}" &>/dev/null
  case "$?" in
    0) return 3 ;;
    2) return 2 ;;
  esac

  local spdx_anchor="${prefix} SPDX-License-Identifier: ${LICENSE_LIB_SPDX_ID}${suffix}"
  if grep -F -x -q -- "${spdx_anchor}" "${path}"; then
    _license_lib_err "${path}: header present but broken. Fix manually"
    return 1
  fi

  local tmp
  tmp="$(mktemp)" || {
    _license_lib_err "mktemp failed"
    return 1
  }
  chmod --reference="${path}" "${tmp}" 2>/dev/null || true

  local first_line=""
  IFS= read -r first_line < "${path}" || true

  {
    if [[ "${first_line}" == '#!'* ]]; then
      printf '%s\n' "${first_line}"
      license_render_header "${path}"
      printf '\n'
      tail -n +2 "${path}" | awk 'NF || seen {print; seen = 1}'
    else
      license_render_header "${path}"
      printf '\n'
      awk 'NF || seen {print; seen = 1}' "${path}"
    fi
  } > "${tmp}"

  if ! mv "${tmp}" "${path}"; then
    _license_lib_err "${path}: failed to write"
    rm -f "${tmp}"
    return 1
  fi
}

license_collect_files() {
  local rel
  while IFS= read -r -d '' rel; do
    license_is_excluded "${rel}" && continue
    license_comment_style "${rel}" &>/dev/null || continue
    printf '%s\0' "${rel}"
  done < <(git -C "${LICENSE_LIB_ROOT_DIR}" ls-files -z)
}
