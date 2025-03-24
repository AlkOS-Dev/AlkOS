#!/usr/bin/env bash
set -e

CLANG_FORMAT_HOOK_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source "$CLANG_FORMAT_HOOK_DIR/../helpers.bash"

# ==========================
# Configuration Constants
# ==========================

# Number of logical CPU cores for parallel processing
NUM_CORES=$(nproc || echo 1)

# Supported C++ file extensions (space-separated)
CPP_EXTENSIONS="cpp cc c h hpp cuh tpp"

# Regular expression pattern to match C++ files (case insensitive)
CPP_PATTERN='\.('"$(echo "$CPP_EXTENSIONS" | tr ' ' '|')"')$'

# Clang-Format style configuration file
CLANG_FORMAT_STYLE="file"

# ==========================
# Main Script Execution
# ==========================

main() {
  if ! command -v clang-format &> /dev/null; then
    log "clang-format is not installed. Please install it before committing"
    exit 1
  fi

  # Use readarray to store null-delimited file names in an array
  readarray -d '' staged_files < <(printf "%s\0" "$@" | grep -z -i -E "$CPP_PATTERN" || true)

  # Exit early if no C++ files are staged
  if [ ${#staged_files[@]} -eq 0 ]; then
    log "No staged C++ files to format"
    exit 0
  fi

  # Display the files that will be formatted
  log "Following files will be formatted:"
  printf '%s\n' "${staged_files[@]}"

  # Run clang-format in parallel using null-delimited input
  printf '%s\0' "${staged_files[@]}" | xargs -0 -P "$NUM_CORES" -n 1 clang-format -style="$CLANG_FORMAT_STYLE" -i || exit 1

  # Re-add the formatted files to the staging area
  printf '%s\0' "${staged_files[@]}" | xargs -0 git add || {
    log "Failed to re-add formatted files to the staging area"
    exit 1
  }
}

main "$@"
