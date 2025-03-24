#!/usr/bin/env bash

HEADER_GUARD_HOOK_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
HEADER_GUARD_HOOK_TARGET_DIR="$HEADER_GUARD_HOOK_DIR/../../.."

source "$HEADER_GUARD_HOOK_DIR/../helpers.bash"

# ==========================
# Configuration Constants
# ==========================

# Number of logical CPU cores for parallel processing
NUM_CORES=$(nproc || echo 1)

# Supported C++ headers extensions (space-separated)
CPP_HEADER_EXTENSIONS="h hpp cuh"

# Regular expression pattern to match C++ headers (case insensitive)
CPP_HEADER_PATTERN='\.('"$(echo "$CPP_HEADER_EXTENSIONS" | tr ' ' '|')"')$'

# ==========================
# Main Script Execution
# ==========================

process_file() {
  local path="$1"
  local rel_path

  rel_path="$(realpath --relative-to="$HEADER_GUARD_HOOK_TARGET_DIR" "$path")"
  header_guard=$(echo "$rel_path" | tr '[:lower:]' '[:upper:]' | tr '/' '_' | tr '.' '_' | tr '-' '_')
  header_guard="${header_guard}_"

  temp_file=$(mktemp)

  # Replace existing header guards if they exist
  awk -v guard="$header_guard" '
    # Track if we found header guard to replace
    BEGIN { found_guard = 0 }

    { lines[NR] = $0 }

    # Match the first #ifndef/#define pair
    /^#ifndef [A-Z_][A-Z0-9_]*$/ {
      old_guard = substr($0, 9)
      lines[NR] = $0
      getline
      if ($0 ~ "^#define " old_guard "$") {
        found_guard = 1
        lines[NR-1] = "#ifndef " guard
        lines[NR] = "#define " guard
      } else {
        lines[NR] = $0
      }
    }

    # Match the ending #endif comment
    /^#endif.*/ && found_guard {
      endif_line = NR
      next
    }

    END {
      if (endif_line) {
        lines[endif_line] = "#endif  // " guard
      }

      for (i = 1; i <= NR; i++) {
        print lines[i]
      }
    }
  ' "$path" > "$temp_file"

  # If no header guards were found, attempt to create them
  if ! grep -q "$header_guard" "$temp_file"; then
    {
      echo "#ifndef $header_guard"
      echo "#define $header_guard"
      echo ""
      cat "$path"
      echo ""
      echo "#endif /* $header_guard */"
    } > "$temp_file"
  fi

  mv "$temp_file" "$path"
}

# Use readarray to store null-delimited file names in an array
readarray -d '' staged_files < <(printf "%s\0" "$@" | grep -z -i -E "$CPP_HEADER_PATTERN")

# Exit early if no C++ headers are staged
if [ ${#staged_files[@]} -eq 0 ]; then
  log "No staged C++ headers to process"
  exit 0
fi

# Display the files that will be processed
log "Following files will be processed:"
printf '%s\n' "${staged_files[@]}"

# Run function in parallel using null-delimited input
export -f process_file
export HEADER_GUARD_HOOK_TARGET_DIR
printf '%s\0' "${staged_files[@]}" | xargs -0 -P "$NUM_CORES" -n 1 bash -c 'process_file "$1"' {} || exit 1

# Re-add the files to the staging area
printf '%s\0' "${staged_files[@]}" | xargs -0 git add || {
  log "Failed to re-add files to the staging area"
  exit 1
}
