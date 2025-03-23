#!/usr/bin/env bash

END_OF_FILE_HOOK_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source "$END_OF_FILE_HOOK_DIR/../helpers.bash"

# ==========================
# Configuration Constants
# ==========================

# Number of logical CPU cores for parallel processing
NUM_CORES=$(nproc || echo 1)

# ==========================
# Main Script Execution
# ==========================

process_file() {
  local filename="$1"

  if [[ ! -s "$filename" ]]; then
    return
  fi

  last_char=$(tail -c 1 "$filename")

  if [[ -n "$last_char" ]]; then
    echo "" >> "$filename"
  else
    temp_file=$(mktemp)

    # Remove trailing empty lines
    sed -e :a -e '/^\n*$/{$d;N;ba' -e '}' -e '/\n$/ba' "$filename" > "$temp_file"

    if ! cmp -s "$filename" "$temp_file"; then
        mv "$temp_file" "$filename"
    fi
  fi
}

# Use readarray to store newline-delimited file names in an array
readarray -t staged_files < <(filter_text_files "$@" || true)

# Exit early if no text files are staged
if [ ${#staged_files[@]} -eq 0 ]; then
  log "No staged text files to format"
  exit 0
fi

# Display the files that will be processed
log "Following files will be processed:"
printf '%s\n' "${staged_files[@]}"

# Run function in parallel using null-delimited input
export -f process_file
printf '%s\0' "${staged_files[@]}" | xargs -0 -P "$NUM_CORES" -n 1 bash -c 'process_file "$1"' {} || exit 1

# Re-add the files to the staging area
printf '%s\0' "${staged_files[@]}" | xargs -0 git add || {
  log "Failed to re-add files to the staging area"
  exit 1
}
