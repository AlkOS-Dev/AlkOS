#!/bin/bash

VERSION_LIB_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
VERSION_LIB_ROOT_DIR="${VERSION_LIB_DIR}/../.."
VERSION_LIB_VERSION_FILE="${VERSION_LIB_ROOT_DIR}/VERSION"
VERSION_LIB_AUTHORS_FILE="${VERSION_LIB_ROOT_DIR}/AUTHORS"
VERSION_LIB_HEADER_PATH="${VERSION_LIB_ROOT_DIR}/generated/include/autogen/version.hpp"

_version_lib_err() {
  echo "version_lib: $*" >&2
}

_version_lib_trim() {
  local s="$1"
  s="${s#"${s%%[![:space:]]*}"}"
  s="${s%"${s##*[![:space:]]}"}"
  printf '%s' "${s}"
}

_version_lib_cstr_escape() {
  local s="$1"
  s="${s//\\/\\\\}"
  s="${s//\"/\\\"}"
  printf '%s' "${s}"
}

# Normalize a boolean-ish value to 1/0. Returns non-zero on an unrecognized value.
_version_lib_normalize_bool() {
  case "${1,,}" in
    1 | true | on | yes) echo 1 ;;
    0 | false | off | no | "") echo 0 ;;
    *) return 1 ;;
  esac
}

version_read() {
  local value=""
  if [[ -f "${VERSION_LIB_VERSION_FILE}" ]]; then
    value="$(tr -d '[:space:]' < "${VERSION_LIB_VERSION_FILE}")"
  fi

  if [[ -z "${value}" ]]; then
    echo "unknown"
    return 1
  fi

  echo "${value}"
}

# Read the AUTHORS file and echo one tab-separated record per contributor:
#     name<TAB>email<TAB>github
# Comment (#) and blank lines are ignored. Returns non-zero if the file is
# missing or holds no valid entries. Reusable by other scripts.
version_read_authors() {
  if [[ ! -f "${VERSION_LIB_AUTHORS_FILE}" ]]; then
    _version_lib_err "authors file not found: ${VERSION_LIB_AUTHORS_FILE}"
    return 1
  fi

  local line name email github rest found=0
  while IFS= read -r line || [[ -n "${line}" ]]; do
    line="$(_version_lib_trim "${line}")"
    [[ -z "${line}" || "${line}" == \#* ]] && continue

    # Expected form: Name <email> (@github)
    if [[ "${line}" != *"<"*">"* ]]; then
      _version_lib_err "malformed author entry (missing <email>): ${line}"
      return 1
    fi

    name="$(_version_lib_trim "${line%%<*}")"
    rest="${line#*<}"
    email="$(_version_lib_trim "${rest%%>*}")"

    github=""
    if [[ "${line}" == *"(@"*")"* ]]; then
      github="${line##*(@}"
      github="${github%%)*}"
    fi

    if [[ -z "${name}" || -z "${email}" ]]; then
      _version_lib_err "malformed author entry (empty name or email): ${line}"
      return 1
    fi

    printf '%s\t%s\t%s\n' "${name}" "${email}" "${github}"
    found=1
  done < "${VERSION_LIB_AUTHORS_FILE}"

  if [[ "${found}" -eq 0 ]]; then
    _version_lib_err "no valid author entries in ${VERSION_LIB_AUTHORS_FILE}"
    return 1
  fi
}

# Parse all version data into the caller's associative array.
# Usage: declare -A info; version_parse info [arch] [build_type] [official]
# Populated keys: name author version major minor patch git_hash git_dirty
#                 version_full build_type arch official
# Returns non-zero on any failure (unreadable/invalid version, missing git, etc.).
version_parse() {
  if [[ -z "${1:-}" ]]; then
    _version_lib_err "version_parse requires a target associative array name"
    return 1
  fi

  local -n _version_out="$1"
  local arch="${2:-unknown}"
  local build_type="${3:-unknown}"
  local official_raw="${4:-0}"

  local official
  if ! official="$(_version_lib_normalize_bool "${official_raw}")"; then
    _version_lib_err "invalid official flag '${official_raw}' (expected 0/1/true/false)"
    return 1
  fi

  local build_date=""
  local build_time=""
  if [[ "${official}" -eq 0 ]]; then
    local now
    if ! now="$(date '+%Y-%m-%d %H:%M:%S')"; then
      _version_lib_err "failed to determine build date/time"
      return 1
    fi
    build_date="${now%% *}"
    build_time="${now#* }"
  fi

  local version
  if ! version="$(version_read)"; then
    _version_lib_err "failed to read version from ${VERSION_LIB_VERSION_FILE}"
    return 1
  fi

  if [[ ! "${version}" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    _version_lib_err "invalid version '${version}' (expected MAJOR.MINOR.PATCH)"
    return 1
  fi

  local major minor patch
  IFS='.' read -r major minor patch <<< "${version}"

  if ! command -v git &>/dev/null; then
    _version_lib_err "git is required but was not found"
    return 1
  fi

  if ! git -C "${VERSION_LIB_ROOT_DIR}" rev-parse --git-dir &>/dev/null; then
    _version_lib_err "not a git repository: ${VERSION_LIB_ROOT_DIR}"
    return 1
  fi

  local git_hash
  if ! git_hash="$(git -C "${VERSION_LIB_ROOT_DIR}" rev-parse --short HEAD 2>/dev/null)" \
    || [[ -z "${git_hash}" ]]; then
    _version_lib_err "failed to read git HEAD"
    return 1
  fi

  local git_dirty=0
  local diff_rc=0
  git -C "${VERSION_LIB_ROOT_DIR}" diff --quiet --ignore-submodules HEAD 2>/dev/null || diff_rc=$?
  if [[ ${diff_rc} -eq 1 ]]; then
    git_dirty=1
  elif [[ ${diff_rc} -ne 0 ]]; then
    _version_lib_err "failed to determine git dirty state (rc=${diff_rc})"
    return 1
  fi

  local version_full
  if version_full="$(git -C "${VERSION_LIB_ROOT_DIR}" describe --tags --dirty=-dirty 2>/dev/null)"; then
    version_full="${version_full#v}"
  else
    version_full="${version}+g${git_hash}"
    [[ ${git_dirty} -eq 1 ]] && version_full="${version_full}-dirty"
  fi

  if [[ "${official}" -eq 1 && "${version_full}" != "${version}" ]]; then
    _version_lib_err \
      "official build requires a clean checkout tagged '${version}' (resolved '${version_full}'); commit everything and run 'git tag v${version}'"
    return 1
  fi

  _version_out[name]="AlkOS"
  _version_out[author]="The AlkOS Authors"
  _version_out[version]="${version}"
  _version_out[major]="${major}"
  _version_out[minor]="${minor}"
  _version_out[patch]="${patch}"
  _version_out[git_hash]="${git_hash}"
  _version_out[git_dirty]="${git_dirty}"
  _version_out[version_full]="${version_full}"
  _version_out[build_type]="${build_type}"
  _version_out[arch]="${arch}"
  _version_out[official]="${official}"
  _version_out[build_date]="${build_date}"
  _version_out[build_time]="${build_time}"
}

version_generate_header() {
  local arch="${1:-unknown}"
  local build_type="${2:-unknown}"
  local official="${3:-0}"
  local output="${4:-${VERSION_LIB_HEADER_PATH}}"

  local -A info
  if ! version_parse info "${arch}" "${build_type}" "${official}"; then
    return 1
  fi

  local authors_tsv
  if ! authors_tsv="$(version_read_authors)"; then
    return 1
  fi

  local authors_cpp="" authors_count=0
  local a_name a_email a_github
  while IFS=$'\t' read -r a_name a_email a_github; do
    [[ -z "${a_name}" ]] && continue
    authors_cpp+="    {\"$(_version_lib_cstr_escape "${a_name}")\", \"$(_version_lib_cstr_escape "${a_email}")\", \"$(_version_lib_cstr_escape "${a_github}")\"},"$'\n'
    authors_count=$((authors_count + 1))
  done <<< "${authors_tsv}"
  authors_cpp="${authors_cpp%$'\n'}"

  local tmp
  if ! tmp="$(mktemp)"; then
    _version_lib_err "failed to create temporary file"
    return 1
  fi

  cat > "${tmp}" << EOF
// This file is generated by the build system. Do not edit manually.
// Source of truth: <repo>/VERSION  (generated via scripts/config/version_lib.bash)

#ifndef GENERATED_INCLUDE_AUTOGEN_VERSION_HPP_
#define GENERATED_INCLUDE_AUTOGEN_VERSION_HPP_

// ------------------------------------------------------------------ macros ---

#define ALKOS_NAME "${info[name]}"
#define ALKOS_AUTHOR "${info[author]}"

#define ALKOS_VERSION_MAJOR ${info[major]}
#define ALKOS_VERSION_MINOR ${info[minor]}
#define ALKOS_VERSION_PATCH ${info[patch]}

#define ALKOS_VERSION_STRING "${info[version]}"
#define ALKOS_VERSION_FULL "${info[version_full]}"

#define ALKOS_GIT_HASH "${info[git_hash]}"
#define ALKOS_GIT_DIRTY ${info[git_dirty]}

#define ALKOS_BUILD_TYPE "${info[build_type]}"
#define ALKOS_ARCH "${info[arch]}"

// 1 for official release builds.
// 0 for internal builds.
#define ALKOS_OFFICIAL_BUILD ${info[official]}

#define ALKOS_BUILD_DATE "${info[build_date]}"
#define ALKOS_BUILD_TIME "${info[build_time]}"

// Number of contributors listed in the AUTHORS file.
#define ALKOS_AUTHORS_COUNT ${authors_count}

// ------------------------------------------------------------- C++ helpers ---

#ifdef __cplusplus

namespace alkos::version
{
inline constexpr unsigned kMajor = ALKOS_VERSION_MAJOR;
inline constexpr unsigned kMinor = ALKOS_VERSION_MINOR;
inline constexpr unsigned kPatch = ALKOS_VERSION_PATCH;

inline constexpr const char* kName      = ALKOS_NAME;
inline constexpr const char* kAuthor    = ALKOS_AUTHOR;
inline constexpr const char* kString    = ALKOS_VERSION_STRING;
inline constexpr const char* kFull      = ALKOS_VERSION_FULL;
inline constexpr const char* kGitHash   = ALKOS_GIT_HASH;
inline constexpr bool        kGitDirty  = (ALKOS_GIT_DIRTY != 0);
inline constexpr const char* kBuildType = ALKOS_BUILD_TYPE;
inline constexpr const char* kArch      = ALKOS_ARCH;
inline constexpr bool        kOfficial  = (ALKOS_OFFICIAL_BUILD != 0);
inline constexpr const char* kBuildDate = ALKOS_BUILD_DATE;
inline constexpr const char* kBuildTime = ALKOS_BUILD_TIME;

struct Author {
    const char* name;
    const char* email;
    const char* github;
};

inline constexpr Author kAuthors[] = {
${authors_cpp}
};
inline constexpr unsigned kAuthorsCount = ALKOS_AUTHORS_COUNT;
}  // namespace alkos::version

#endif  // __cplusplus

#endif  // GENERATED_INCLUDE_AUTOGEN_VERSION_HPP_
EOF

  if ! mkdir -p "$(dirname "${output}")"; then
    _version_lib_err "failed to create output directory for ${output}"
    rm -f "${tmp}"
    return 1
  fi

  if [[ ! -f "${output}" ]] || ! cmp -s "${tmp}" "${output}"; then
    if ! mv "${tmp}" "${output}"; then
      _version_lib_err "failed to write ${output}"
      rm -f "${tmp}"
      return 1
    fi
    chmod 0644 "${output}"
  else
    rm -f "${tmp}"
  fi
}
