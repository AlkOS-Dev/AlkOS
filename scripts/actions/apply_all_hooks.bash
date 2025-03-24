#!/usr/bin/env bash

APPLY_ALL_HOOKS_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source "$APPLY_ALL_HOOKS_DIR/../git-hooks/helpers.bash"
source "$APPLY_ALL_HOOKS_DIR/../utils/conf_handlers.bash"
source_conf_file

# ===============================
# Configuration Variables
# ===============================

declare -a APPLY_ALL_HOOKS_EXCLUDE_PATHS=(
    "$(conf_file_read_var "CONF_BUILD_DIR" "$APPLY_ALL_HOOKS_DIR/../../build")"
    "$(conf_file_read_var "CONF_TOOL_DIR" "$APPLY_ALL_HOOKS_DIR/../../tools")"
    "$APPLY_ALL_HOOKS_DIR/../../.git"
)

TARGET_FOLDER="$APPLY_ALL_HOOKS_DIR/../.."
HOOKS_DIR="$APPLY_ALL_HOOKS_DIR/../git-hooks/hooks"
NUM_CORES=$(nproc || echo 1)

# ===============================
# Script Execution
# ===============================

main() {
  echo "Applying all hooks"

  # Find all text files
  readarray -t files < <(find_text_files "$TARGET_FOLDER" "${APPLY_ALL_HOOKS_EXCLUDE_PATHS[@]}")

  # Remove files excluded from git
  tmp=()
  for file in "${files[@]}"; do
      if ! git check-ignore -q "$file"; then
          tmp+=("$file")
      fi
  done
  files=("${tmp[@]}")

  # Run all hooks
  for hook in "$HOOKS_DIR"/*; do
      if [ -f "$hook" ]; then
        runner "$hook" "${files[@]}"
      fi
  done

  echo "All hooks executed successfully"
}

main "$@"
