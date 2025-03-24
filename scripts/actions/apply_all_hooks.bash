#!/usr/bin/env bash

APPLY_ALL_HOOKS_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
source "$APPLY_ALL_HOOKS_DIR/../git-hooks/helpers.bash"

declare -a APPLY_ALL_HOOKS_EXCLUDE_PATHS=(
    "$APPLY_ALL_HOOKS_DIR/../../build"
    "$APPLY_ALL_HOOKS_DIR/../../tools"
    "$APPLY_ALL_HOOKS_DIR/../../alkos/cmake-*"
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

FILES="$(find_text_files "$TARGET_FOLDER" "${APPLY_ALL_HOOKS_EXCLUDE_PATHS[@]}")"

for hook in "$HOOKS_DIR"/*; do
    if [ -f "$hook" ]; then
        echo "Executing hook: $(basename "$hook")"
        echo "$FILES" | xargs -d '\n' -P "$NUM_CORES" -I {} bash "$hook" "{}"
    fi
done

echo "All hooks executed successfully"
