#!/bin/env bash

log () {
    echo "[pre-commit hook] $1"
}

runner () {
  local basename
  basename=$(basename "$1")
  local hook="$1"
  shift

  log "Running $basename"
  if ! bash -c "$hook"' "$@"' -- "$@"; then
      log "$basename failed"
      exit 1
  fi
}

find_text_files() {
  local target_path="$1"; shift
  local exluded_paths=("$@")

  # Construct the find command with excluded paths
  find_command=(find "$target_path" -print0 \( )
  for path in "${exluded_paths[@]}"; do
      find_command+=(-path "$path" -o)
  done
  unset 'find_command[${#find_command[@]}-1]'
  find_command+=(\) -prune -o -name '*.*')

  "${find_command[@]}" | xargs -0 -n 1 file 2> /dev/null | grep "text" | cut -d ":" -f1
}

filter_text_files() {
  local files
  files=$(printf "%s\0" "$@" | xargs -0 -n 1 file 2> /dev/null | grep 'text' | cut -d ":" -f1)
  if [[ -n "$files" ]]; then
    echo "$files"
  else
    return 1
  fi
}
