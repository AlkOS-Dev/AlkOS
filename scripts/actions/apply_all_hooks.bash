#!/usr/bin/env bash

APPLY_ALL_HOOKS_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source "$APPLY_ALL_HOOKS_DIR/../git-hooks/helpers.bash"

declare -a APPLY_ALL_HOOKS_EXCLUDE_PATHS=(
    "$APPLY_ALL_HOOKS_DIR/../../build"
    "$APPLY_ALL_HOOKS_DIR/../../tools"
    "$APPLY_ALL_HOOKS_DIR/../../alkos/cmake-*"
    "$APPLY_ALL_HOOKS_DIR/../../.git"
    "$APPLY_ALL_HOOKS_DIR/../../.idea"
)

# ===============================
# Configuration Variables
# ===============================

TARGET_FOLDER="$APPLY_ALL_HOOKS_DIR/../.."
HOOKS_DIR="$APPLY_ALL_HOOKS_DIR/../git-hooks/hooks"
NUM_CORES=$(nproc || echo 1)

# ===============================
# Script Execution
# ===============================

echo "Applying all hooks"

# Find all text files
readarray -t files < <(find_text_files "$TARGET_FOLDER" "${APPLY_ALL_HOOKS_EXCLUDE_PATHS[@]}")

# Remove files excluded from git
for file in "${files[@]}"; do
    if git check-ignore -q "$file"; then
        files=("${files[@]/$file}")
    fi
done

# Run all hooks
for hook in "$HOOKS_DIR"/*; do
    if [ -f "$hook" ]; then
      runner "$hook" "${files[@]}"
    fi
done

echo "All hooks executed successfully"
