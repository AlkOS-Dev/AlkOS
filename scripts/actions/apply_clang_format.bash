#!/usr/bin/env bash

# ===============================
# Configuration Variables
# ===============================

TARGET_FOLDER="."
FILE_EXTENSIONS=("cpp" "cc" "c" "h" "hpp" "tpp")
CLANG_FORMAT_STYLE="file"

# ===============================
# Script Execution
# ===============================

num_cores=$(nproc)
echo "Using $num_cores CPU cores for formatting."

# Construct the find command with multiple -name options
find_command=(find "$TARGET_FOLDER" -type f \( )
for ext in "${FILE_EXTENSIONS[@]}"; do
    find_command+=(-name "*.$ext" -o)
done
unset 'find_command[${#find_command[@]}-1]'
find_command+=(\) )

echo "Searching in '$TARGET_FOLDER' for files with extensions: ${FILE_EXTENSIONS[*]}"

# Execute the find command and pipe to xargs
"${find_command[@]}" | xargs -d '\n' -P "$num_cores" -I {} clang-format -i -style="$CLANG_FORMAT_STYLE" "{}"

echo "Formatting completed."

