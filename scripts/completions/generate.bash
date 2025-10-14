#!/usr/bin/env bash

readonly GENCOMP_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
readonly GENCOMP_SCRIPT_PATH="${GENCOMP_DIR}/$(basename "$0")"
readonly GENCOMP_SCRIPTS_BASE_DIR="$(realpath "${GENCOMP_DIR}/..")"
readonly GENCOMP_OUTPUT_DIR="${GENCOMP_DIR}/output"

declare -Ar GENCOMP_GENERATED_OUTPUT_PATH=(
    [bash]="${GENCOMP_OUTPUT_DIR}/bash_completion.generated.bash"
    [zsh]="${GENCOMP_OUTPUT_DIR}/zsh_completion.generated.zsh"
    [fish]="${GENCOMP_OUTPUT_DIR}/fish_completion.generated.fish"
)

declare -Ar GENCOMP_TARGET_LINK_PATH=(
    [bash]="$HOME/.local/share/bash-completion/completions/alkos"
    [zsh]="$HOME/.local/share/zsh/site-functions/_alkos"
    [fish]="$HOME/.config/fish/completions/alkos.fish"
)

declare -Ar GENCOMP_CONFIG_FILE=(
    [bash]="$HOME/.bash_completion"
    [zsh]="$HOME/.zshrc"
    [fish]="$HOME/.config/fish/config.fish"
)

declare -ar GENCOMP_EXCLUDED_DIRS=(
    "${GENCOMP_DIR}"
    "${GENCOMP_SCRIPTS_BASE_DIR}/actions"
    "${GENCOMP_SCRIPTS_BASE_DIR}/cicd"
    "${GENCOMP_SCRIPTS_BASE_DIR}/git-hooks"
    "${GENCOMP_SCRIPTS_BASE_DIR}/tests"
    "${GENCOMP_SCRIPTS_BASE_DIR}/utils"
)

source "${GENCOMP_DIR}/../utils/argparse.bash"
source "${GENCOMP_DIR}/../utils/helpers.bash"

GENCOMP_SCRIPTS_LIST=()

parse_args() {
    argparse_init "${GENCOMP_SCRIPT_PATH}" "Generate bash completion scripts for all .bash files in the repository"
    argparse_add_positional "shell" "Shell type" true "bash|zsh|fish"
    argparse_add_option "r|run" "Generate completion scripts" false false "" "flag"
    argparse_add_option "e|enable" "Enable scripts autocompletion" false false "" "flag"
    argparse_add_option "d|disable" "Disable scripts autocompletion" false false "" "flag"
    argparse_parse "$@"
}

validate_args() {
    if [[ "$(argparse_get "e|enable")" == "true" && "$(argparse_get "d|disable")" == "true" ]]; then
        dump_error "--enable and --disable cannot be used together."
    fi

    if [[ "$(argparse_get "r|run")" != "true" && "$(argparse_get "e|enable")" != "true" && "$(argparse_get "d|disable")" != "true" ]]; then
        dump_error "At least one of --run, --enable, or --disable must be specified."
    fi
}

add_link() {
    local script="$1"
    local target_link="$2"
    local script_name="$(basename "$script")"

    pretty_info "Creating symlink..."
    if [[ -L "$target_link" ]]; then
        if [[ "$(readlink -f "$target_link")" == "$script" ]]; then
            pretty_info "Symlink for ${script_name} already exists"
            return
        else
            pretty_info "Symlink for ${script_name} exists but points to a different file. Updating..."
            base_runner "Failed to update symlink for ${script_name}" false \
                rm "$target_link"
        fi
    elif [[ -e "$target_link" ]]; then
        dump_error "A file named $(basename "$target_link") already exists and is not a symlink"
    fi

    base_runner "Failed to create symlink for ${script_name}" false \
        ln -s "$script" "$target_link"
    pretty_success "Created symlink for ${script_name}"
}

remove_link() {
    local script="$1"
    local target_link="$2"
    local script_name="$(basename "$script")"

    pretty_info "Removing Fish completion symlink..."
    if [[ -L "$target_link" ]]; then
        base_runner "Failed to remove symlink for ${script_name}" false \
            rm "$target_link"
        pretty_success "Removed symlink for ${script_name}"
    elif [[ -e "$target_link" ]]; then
        dump_error "A file named $(basename "$target_link") exists and is not a symlink"
    else
        pretty_info "No symlink for ${script_name} found"
    fi
}

get_config_line() {
    local shell="$1"
    case "$shell" in
        bash)
            echo "source \"${GENCOMP_TARGET_LINK_PATH[$shell]}\" 2>/dev/null || true"
            ;;
        fish)
            echo "source \"${GENCOMP_TARGET_LINK_PATH[$shell]}\" 2>/dev/null || true"
            ;;
    esac
}

add_local_zsh_fpath() {
    local config_file="${GENCOMP_CONFIG_FILE[zsh]}"
    local fpath_line="fpath=(~/.local/share/zsh/site-functions \$fpath)"

    pretty_info "Adding local zsh fpath..."

    # Create config file if it doesn't exist
    if [[ ! -f "$config_file" ]]; then
        touch "$config_file"
    fi

    # Add fpath line if not already present
    if ! grep -qF "$fpath_line" "$config_file"; then
        # Prepend fpath at the beginning of the file
        local temp_file=$(mktemp)
        {
            echo "# User local zsh completions"
            echo "$fpath_line"
            echo ""
            cat "$config_file"
        } > "$temp_file" && mv "$temp_file" "$config_file"
        pretty_success "Added local zsh fpath"
    else
        pretty_info "Config already contains the local zsh fpath"
    fi
}

add_completion_sourcing() {
    local shell="$1"
    local config_file="${GENCOMP_CONFIG_FILE[$shell]}"
    local config_name="$(basename "$config_file")"
    local config_line="$(get_config_line "$shell")"

    [[ -z "$config_file" || -z "$config_line" ]] && return

    pretty_info "Adding completion sourcing to $config_name..."
    
    # Create config file if it doesn't exist
    if [[ ! -f "$config_file" ]]; then
        touch "$config_file"
    fi

    # Add sourcing line if not already present
    if ! grep -qF "$config_line" "$config_file"; then
        # Add newline if file is not empty and doesn't end with newline
        if [[ -s "$config_file" && "$(tail -c2 "$config_file")" != "" ]]; then
            echo "" >> "$config_file"
        fi
        {
            echo "# AlkOS completions"
            echo "$config_line"
        } >> "$config_file"
        pretty_success "Added completion sourcing to $config_name"
    else
        pretty_info "$config_name already contains the completion sourcing"
    fi
}

remove_completion_sourcing() {
    local shell="$1"
    local config_file="${GENCOMP_CONFIG_FILE[$shell]}"

    # Skip if no config file is needed
    [[ -z "$config_file" ]] && return

    local config_name="$(basename "$config_file")"
    local config_line="$(get_config_line "$shell")"

    # Check if config file exists
    if [[ ! -f "$config_file" ]]; then
        pretty_info "$config_name file not found"
        return
    fi

    pretty_info "Removing completion sourcing from $config_name..."
    
    # Remove the completion sourcing block
    if grep -qF "$config_line" "$config_file"; then
        # For bash and fish, remove the comment block
        sed -i '/# AlkOS completions/{
                    N
                    \#'"$config_line"'$#{
                        :loop
                        $!{
                            N
                            /^[[:blank:]]*$/{
                                b loop
                            }
                            d
                        }
                        d
                    }
                }' "$config_file"
        pretty_success "Removed completion sourcing from $config_name"
    else
        pretty_info "$config_name doesn't contain the completion sourcing"
    fi
}

enable_completion() {
    local shell="$1"
    local script="${GENCOMP_GENERATED_OUTPUT_PATH[$shell]}"
    local target_link="${GENCOMP_TARGET_LINK_PATH[$shell]}"

    # Ensure the given shell is installed
    if ! command -v "$shell" &> /dev/null; then
        dump_error "The specified shell '$shell' is not installed"
    fi

    case "$shell" in
        bash)
            # Create bash completions directory if it doesn't exist
            local bash_completions_dir="$(dirname "$target_link")"
            if [[ ! -d "$bash_completions_dir" ]]; then
                base_runner "Failed to create bash completions directory" false \
                    mkdir -p "$bash_completions_dir"
                pretty_info "Created directory: $bash_completions_dir"
            fi

            add_link "$script" "$target_link"
            add_completion_sourcing "$shell"
            ;;
        zsh)
            # Create zsh completions directory if it doesn't exist
            local zsh_completions_dir="$(dirname "$target_link")"
            if [[ ! -d "$zsh_completions_dir" ]]; then
                base_runner "Failed to create zsh completions directory" false \
                    mkdir -p "$zsh_completions_dir"
                pretty_info "Created directory: $zsh_completions_dir"
            fi

            add_link "$script" "$target_link"

            # Ask user to add fpath if not already present
            user_choice "Do you want to add the local zsh fpath to your .zshrc?" add_local_zsh_fpath
            if [[ $? -ne 0 ]]; then
                pretty_warn "Please remember to add the local zsh fpath manually to your .zshrc:"
                pretty_warn 'fpath=(~/.local/share/zsh/site-functions $fpath)'
            fi
            ;;
        fish)
            # Create Fish completions directory if it doesn't exist
            local fish_completions_dir="$(dirname "$target_link")"
            if [[ ! -d "$fish_completions_dir" ]]; then
                base_runner "Failed to create Fish completions directory" false \
                    mkdir -p "$fish_completions_dir"
                pretty_info "Created directory: $fish_completions_dir"
            fi

            add_link "$script" "$target_link"
            add_completion_sourcing "$shell"
            ;;
    esac
}

disable_completion() {
    local shell="$1"
    local script="${GENCOMP_GENERATED_OUTPUT_PATH[$shell]}"
    local target_link="${GENCOMP_TARGET_LINK_PATH[$shell]}"

    # Ensure the given shell is installed
    if ! command -v "$shell" &> /dev/null; then
        dump_error "The specified shell '$shell' is not installed"
    fi

    case "$shell" in
        bash)
            remove_link "$script" "$target_link"
            remove_completion_sourcing "$shell"
            ;;
        zsh)
            remove_link "$script" "$target_link"
            ;;
        fish)
            remove_link "$script" "$target_link"
            remove_completion_sourcing "$shell"
            ;;
    esac
}

synthesize_find_command() {
    local target_path="$1"; shift
    local excluded_paths=("$@")

    # Construct the find command with excluded paths
    find_command=(find "$target_path" \\\( )
    for path in "${excluded_paths[@]}"; do
        find_command+=(-path "$path" -o)
    done
    unset 'find_command[${#find_command[@]}-1]'
    find_command+=(\\\) -prune -o -type f -name '"*.bash"' -print0)

    echo "${find_command[*]}"
}

parse_args_exist() {
    local script_path="$1"
    if grep -q '^parse_args()' "$script_path"; then
        return 0
    else
        return 1
    fi
}

filter_scripts() {
    local paths=("$@")
    local filtered_paths=()

    for script in "${paths[@]}"; do
        if parse_args_exist "$script"; then
            filtered_paths+=("$script")
        else
            pretty_warn "Skipping ${script} as it does not contain a parse_args function." >&2
        fi
    done

    echo "${filtered_paths[@]}"
}

get_script_argparse() {
    local script_path="$1"
    # Extract the parse_args function definition directly from the script
    local parse_args_def="$(awk '/^parse_args\(\)/ {flag=1} flag; /^}/ {if(flag){flag=0; print; exit}}' "$script_path")"

    echo "$parse_args_def" | grep 'argparse_' | grep -v 'argparse_parse\|argparse_get'
}

generate_bash_completion() {
  local output_file="${GENCOMP_GENERATED_OUTPUT_PATH[bash]}"

  local paths
  read -ra paths <<< "$(filter_scripts "$@")"

  local func_name="_comp_alkos"
  local script_names=($(printf '%s\0' "${paths[@]}" | xargs -0 -n1 basename))

  {
    echo "#!/usr/bin/env bash"
    echo "# Bash completion for AlkOS scripts"
    echo "# Do not try to edit this by hand!"
    echo
    echo "${func_name}() {"
    echo "    local -a COMP_OPTIONS COMP_POSITIONAL_NAMES"
    echo "    local -A COMP_DESCRIPTIONS COMP_CHOICES COMP_TYPES"
    echo
    echo '    case "$(basename "${COMP_WORDS[0]}")" in'
    for script in "${paths[@]}"; do
      echo "        $(basename "$script"))"
      echo "$(
          # Populate argument definitions
          eval "$(get_script_argparse "$script")"

          # Generate the definitions
          if [[ ${#ARGPARSE_OPTIONS[@]} -gt 0 ]]; then
              echo "            COMP_OPTIONS=("
              for key in "${!ARGPARSE_OPTIONS[@]}"; do
                  echo "                \"$key\""
              done
              echo "            )"
          fi

          if [[ ${#ARGPARSE_DESCRIPTIONS[@]} -gt 0 ]]; then
              echo "            COMP_DESCRIPTIONS=("
              for key in "${!ARGPARSE_DESCRIPTIONS[@]}"; do
                  # Escape double quotes in descriptions
                  desc="${ARGPARSE_DESCRIPTIONS[$key]//\"/\\\"}"
                  echo "                [\"$key\"]=\"$desc\""
              done
              echo "            )"
          fi

          if [[ ${#ARGPARSE_CHOICES[@]} -gt 0 ]]; then
              echo "            COMP_CHOICES=("
              for key in "${!ARGPARSE_CHOICES[@]}"; do
                  echo "                [\"$key\"]=\"${ARGPARSE_CHOICES[$key]}\""
              done
              echo "            )"
          fi

          if [[ ${#ARGPARSE_TYPES[@]} -gt 0 ]]; then
              echo "            COMP_TYPES=("
              for key in "${!ARGPARSE_TYPES[@]}"; do
                  echo "                [\"$key\"]=\"${ARGPARSE_TYPES[$key]}\""
              done
              echo "            )"
          fi

          if [[ ${#ARGPARSE_SEPARATORS[@]} -gt 0 ]]; then
              echo "            COMP_SEPARATORS=("
              for key in "${!ARGPARSE_SEPARATORS[@]}"; do
                  echo "                [\"$key\"]=\"${ARGPARSE_SEPARATORS[$key]}\""
              done
              echo "            )"
          fi

          if [[ ${#ARGPARSE_POSITIONAL_NAMES[@]} -gt 0 ]]; then
              echo "            COMP_POSITIONAL_NAMES=("
              for name in "${ARGPARSE_POSITIONAL_NAMES[@]}"; do
                  echo "                \"$name\""
              done
              echo "            )"
          fi
          )"
      echo "            ;;"
    done
    echo "    esac"
    echo
    echo "    source \"${GENCOMP_DIR}/helpers/bash_helper.bash\" # Lazy load"
    echo "    ${func_name}_helper"
    echo "}"
    echo
    echo "complete -F $func_name ${script_names[0]} \\"
    for ((i = 1; i < ${#script_names[@]} - 1; i++)); do
        echo "                        ${script_names[i]} \\"
    done
    echo "                        ${script_names[-1]}"
  } > "$output_file"
}

generate_zsh_completion() {
    local output_file="${GENCOMP_GENERATED_OUTPUT_PATH[zsh]}"

    local paths
    read -ra paths <<< "$(filter_scripts "$@")"

    local func_name="_alkos"
    local script_names=($(printf '%s\0' "${paths[@]}" | xargs -0 -n1 basename))

    {
        echo "#compdef ${script_names[*]}"
        echo ""
        echo "# Zsh completion for AlkOS scripts"
        echo "# Do not try to edit this by hand!"
        echo ""
        echo "${func_name}() {"
        echo "    local common=("
        echo '        "(- 1 *)"{-h,--help}"[Show help message]"'
        echo "    )"
        echo ""
        echo "    case \"\$(basename \"\${words[1]}\")\" in"

        for script in "${paths[@]}"; do
            local script_name="$(basename "$script")"
            echo "        ${script_name})"
            echo "$(
                # Populate script definitions
                eval "$(get_script_argparse "$script")"

                local completion_lines=()
                completion_lines+=("            _arguments -S \$common \\")

                # Process options definitions
                for key in "${!ARGPARSE_OPTIONS[@]}"; do
                    local short_opt long_opt
                    _parse_option_variants "$key" short_opt long_opt

                    local option_spec=""
                    local description="${ARGPARSE_DESCRIPTIONS[$key]}"
                    local choices="${ARGPARSE_CHOICES[$key]}"
                    local type="${ARGPARSE_TYPES[$key]}"

                    # Build the option specification
                    if [[ -n "$short_opt" && -n "$long_opt" ]]; then
                        option_spec="{-$short_opt,--$long_opt}"
                    elif [[ -n "$short_opt" ]]; then
                        option_spec="-$short_opt"
                    else
                        option_spec="--$long_opt"
                    fi

                    # Add description if available
                    if [[ -n "$description" ]]; then
                        option_spec+="\"[${description}]"
                    fi

                    # Add choices or file completion based on type
                    if [[ "$type" != "flag" && -n "$choices" ]]; then
                        option_spec+=":value:(${choices//|/ })"
                    elif [[ "$type" == "file" ]]; then
                        option_spec+=":filepath:_files"
                    elif [[ "$type" == "directory" ]]; then
                        option_spec+=':directory:_path_files -g \"*(/)\"'
                    fi

                    option_spec+='"'

                    completion_lines+=("                ${option_spec} \\")
                done

                # Process positional arguments
                for ((i = 1; i <= ${#ARGPARSE_POSITIONAL_NAMES[@]}; i++)); do
                    local pos_name="${ARGPARSE_POSITIONAL_NAMES[i - 1]}"
                    local type="${ARGPARSE_TYPES[$pos_name]}"

                    local choices="${ARGPARSE_CHOICES[$pos_name]}"
                    if [[ "$type" != "flag" && -n "$choices" ]]; then
                        completion_lines+=("                '$i:$pos_name:(${choices//|/ })' \\")
                    elif [[ "$type" == "file" ]]; then
                        completion_lines+=("                '$i:$pos_name:_files' \\")
                    elif [[ "$type" == "directory" ]]; then
                        completion_lines+=("                '$i:$pos_name:_path_files -g \"*(/)\"' \\")
                    else
                        completion_lines+=("                '$i:$pos_name:' \\")
                    fi
                done

                # Remove trailing backslash from the last line
                if [[ ${#completion_lines[@]} -gt 0 ]]; then
                    local last_index=$((${#completion_lines[@]} - 1))
                    completion_lines[last_index]="${completion_lines[last_index]% \\}"
                fi

                # Output all completion lines
                printf '%s\n' "${completion_lines[@]}"
                echo "            ;;"
            )"
        done
        echo "    esac"
        echo "}"
        echo ""
        echo "if [[ \"\${funcstack[1]}\" == \"$func_name\" ]]; then"
        echo "    $func_name"
        echo "else"
        echo "    compdef $func_name ${script_names[0]} \\"
        for ((i=1; i<${#script_names[@]}-1; i++)); do
            echo "                   ${script_names[i]} \\"
        done
        echo "                   ${script_names[-1]}"
        echo "fi"
    } > "$output_file"
}

generate_fish_completion() {
    local output_file="${GENCOMP_GENERATED_OUTPUT_PATH[fish]}"

    local paths
    read -ra paths <<< "$(filter_scripts "$@")"

    local script_names=($(printf '%s\0' "${paths[@]}" | xargs -0 -n1 basename))

    {
        echo "# Fish completion for AlkOS scripts"
        echo "# Do not try to edit this by hand!"
        echo ""

        for script in "${paths[@]}"; do
            local script_name="$(basename "$script")"
            echo "# Completion for $script_name"
            echo "complete -c $script_name -f"
            echo "$(
                # Populate script definitions
                eval "$(get_script_argparse "$script")"

                # Add help option
                ARGPARSE_OPTIONS["h|help"]="h|help"
                ARGPARSE_DESCRIPTIONS["h|help"]="Show help message"
                ARGPARSE_TYPES["h|help"]="flag"

                # Generate completions for each option
                for key in "${!ARGPARSE_OPTIONS[@]}"; do
                    local short_opt long_opt
                    _parse_option_variants "$key" short_opt long_opt

                    local description="${ARGPARSE_DESCRIPTIONS[$key]}"
                    local choices="${ARGPARSE_CHOICES[$key]}"
                    local type="${ARGPARSE_TYPES[$key]}"

                    local completion_line="complete -c $script_name"

                    # Add short option if present
                    # Using `-o` instead of `-s` since combined short options are not supported
                    if [[ -n "$short_opt" ]]; then
                        completion_line="$completion_line -o $short_opt"
                    fi

                    # Add long option if exists
                    if [[ -n "$long_opt" ]]; then
                        completion_line="$completion_line -l $long_opt"
                    fi

                    # Add description
                    if [[ -n "$description" ]]; then
                        completion_line="$completion_line -d '$description'"
                    fi

                    # Add argument requirements and choices
                    if [[ "$type" != "flag" ]]; then
                        completion_line="$completion_line -r"
                        if [[ -n "$choices" ]]; then
                            completion_line="$completion_line -a '${choices//|/ }'"
                        elif [[ "$type" == "file" ]]; then
                            completion_line="$completion_line -F"
                        elif [[ "$type" == "directory" ]]; then
                            completion_line="$completion_line -a '(__fish_complete_directories)'"
                        fi
                    fi

                    echo "$completion_line"
                done

                # Generate completions for positional arguments
                for ((i = 1; i <= ${#ARGPARSE_POSITIONAL_NAMES[@]}; i++)); do
                    local pos_name="${ARGPARSE_POSITIONAL_NAMES[i - 1]}"
                    local type="${ARGPARSE_TYPES[$pos_name]}"
                    local choices="${ARGPARSE_CHOICES[$pos_name]}"

                    local completion_line="complete -c $script_name"

                    # Check if this is the nth argument
                    completion_line="$completion_line -n '__fish_is_nth_token $i'"

                    if [[ "$type" != "flag" && -n "$choices" ]]; then
                        completion_line="$completion_line -a '${choices//|/ }'"
                    elif [[ "$type" == "file" ]]; then
                        completion_line="$completion_line -F"
                    elif [[ "$type" == "directory" ]]; then
                        completion_line="$completion_line -a '(__fish_complete_directories)'"
                    fi

                    echo "$completion_line"
                done
            )"
            echo ""
        done
    } > "$output_file"
}

main() {
    parse_args "$@"
    validate_args

    local shell="$(argparse_get "shell")"
    local output_path="${GENCOMP_GENERATED_OUTPUT_PATH[$shell]}"

    if [[ "$(argparse_get "r|run")" == "true" ]]; then
        # Get all .bash scripts excluding certain directories
        while IFS= read -r -d '' script; do
            GENCOMP_SCRIPTS_LIST+=("$script")
        done < <(eval "$(synthesize_find_command "${GENCOMP_SCRIPTS_BASE_DIR}" "${GENCOMP_EXCLUDED_DIRS[@]}")")

        # Generate completion script
        pretty_info "Generating ${shell} completion script..."
        case "$shell" in
            bash)
                generate_bash_completion "${GENCOMP_SCRIPTS_LIST[@]}"
                ;;
            zsh)
                generate_zsh_completion "${GENCOMP_SCRIPTS_LIST[@]}"
                ;;
            fish)
                generate_fish_completion "${GENCOMP_SCRIPTS_LIST[@]}"
                ;;
        esac

        # Set permissions to the generated script
        chmod 644 "${GENCOMP_GENERATED_OUTPUT_PATH[$shell]}"

        pretty_success "Completion script generated at: $(realpath "$output_path")"
    fi

    if [[ "$(argparse_get "e|enable")" == "true" ]]; then
        # Ensure the completion script exists
        if [[ ! -f "$output_path" ]]; then
            dump_error "Completion script not found. Please run with --run first."
        fi

        enable_completion "$shell"
    elif [[ "$(argparse_get "d|disable")" == "true" ]]; then
        disable_completion "$shell"
    fi
}

main "$@"
