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
  find "$1" -type f -print0 | xargs -0 -n 1 file 2> /dev/null | grep "text" | cut -d ":" -f1
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
