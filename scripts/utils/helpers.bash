#!/bin/bash

HELPERS_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
HELPERS_LOG_FILE="/tmp/alkOS_build.log"

source "${HELPERS_SCRIPT_DIR}/pretty_print.bash"

dump_error() {
    pretty_error "$1" >&2
    pretty_error "Use -h or --help for usage information." >&2

    exit 1
}

assert_always() {
    dump_error "ASSERT failed in ${BASH_SOURCE[1]} at line ${BASH_LINENO[0]}"
}

assert_argument_provided() {
    if [ -z "$1" ]; then
        dump_error "ASSERT: argument was not provided, failed in ${BASH_SOURCE[1]} at line ${BASH_LINENO[0]}"
    fi
}

base_runner() {
    assert_argument_provided "$1"
    local dump_info="$1"
    shift
    assert_argument_provided "$1"
    local verbose="$1"
    shift

    pretty_info "Running: $@" >&2

    if [ "$verbose" = true ]; then
        { "$@" 2>&1 | tee "$HELPERS_LOG_FILE"; } && exit_code=${PIPESTATUS[0]} || exit_code=${PIPESTATUS[0]}
    else
        { "$@" > "$HELPERS_LOG_FILE" 2>&1; } && exit_code=$? || exit_code=$?
    fi

    if [ $exit_code -ne 0 ]; then
        dump_error "${dump_info}"
    fi
}


attempt_runner() {
    assert_argument_provided "$1"
    local dump_info="$1"
    shift
    assert_argument_provided "$1"
    local verbose="$1"
    shift

    pretty_info "Running: $@" >&2

    local ret
    if [ "$verbose" = true ]; then
        { "$@" 2>&1 | tee "$HELPERS_LOG_FILE"; } && ret=${PIPESTATUS[0]} || ret=${PIPESTATUS[0]}
    else
        { "$@" > "$HELPERS_LOG_FILE" 2>&1; } && ret=$? || ret=$?
    fi

    if [ ${ret} -ne 0 ]; then
        pretty_error "${dump_info}"
    fi

    return ${ret}
}

check_is_in_env_path() {
    assert_argument_provided "$1"

    local new_path="$1"

    pretty_info "Checking if ${new_path} is in PATH"

    IFS=':' read -ra path_array <<< "${PATH}"

    local existing_path
    for existing_path in "${path_array[@]}"; do
        if [ "$existing_path" = "${new_path}" ]; then
            pretty_info "Path: ${new_path} already exists in PATH"
            return 0
        fi
    done

    pretty_info "Path: ${new_path} does not exist in PATH"
    return 1
}

add_to_user_env_path() {
    assert_argument_provided "$1"
    assert_argument_provided "$2"

    local new_path="$1"
    local is_added="$2"
    pretty_info "Adding ${new_path} to PATH"

    if [ "${is_added}" = false ]; then
        echo "export PATH=\"${new_path}:\$PATH\"" >> ~/.bashrc
        echo "export PATH=\"${new_path}:\$PATH\"" >> ~/.profile
        pretty_success "Path: ${new_path} added to PATH"
    else
        pretty_info "Path: ${new_path} already exists in PATH"
    fi
}

get_supported_distro_name() {
    cat /etc/*-release | tr [:upper:] [:lower:] | grep -Poi '(arch|ubuntu)' | uniq
}

snake_case_to_pascal_case() {
    assert_argument_provided "$1"
    local input="$1"
    input=$(echo "$input" | sed 's/^\(.\)/\U\1/')               # Capitalize first letter
    input=$(echo "$input" | sed 's/_\([a-z]\)/\U\1/g')          # Capitalize letters after underscores
    echo "$input"
}

convert_to_upper_case() {
    assert_argument_provided "$1"
    local input="$1"
    echo "$input" | tr '[:lower:]' '[:upper:]'
}

strip_quotes() {
    assert_argument_provided "$1"
    local input="$1"
    echo "$input" | sed 's/^"\(.*\)"$/\1/' | sed "s/^'\(.*\)'$/\1/"
}

require_root_privileges() {
    if [ "$EUID" -ne 0 ]; then
        dump_error "This operation requires root privileges. Please run as root or use sudo."
    fi
}

# https://unix.stackexchange.com/a/438712
run_with_sudo() {
    assert_argument_provided "$1"
    local arg="$1"
    if [ $(type -t $arg) = function ]
    then
        shift && command sudo bash -c "
            source '${HELPERS_SCRIPT_DIR}/helpers.bash'
            $(declare -f $arg)
            $arg $*
        "
    elif [ $(type -t $arg) = alias ]
    then
            alias sudo='\sudo '
            eval "sudo $@"
    else
            command sudo "$@"
    fi
}

user_choice() {
    assert_argument_provided "$1"
    local prompt="$1"
    shift

    read -r -p "$prompt (y/N): " choice
    case "$choice" in
        y|Y)
            "$@"
            return 0
            ;;
        '') ;&
        n|N) return 1 ;;
        *) return 2 ;;
    esac
}

prompt_to_execute() {
  local root_required="$1"
  shift

  local func_name="$1"
  shift
  pretty_info "The following function will be executed: ${func_name} $*"
  if [ "$(type -t $func_name)" = function ]; then
      pretty_info "Function content:"
      declare -f "$func_name"
  fi
  if [ "$root_required" = true ]; then
      pretty_warn "This function requires root privileges."
  fi

  if [[ "$root_required" = true ]]; then
      user_choice "Do you want to proceed with root privileges?" \
          run_with_sudo "$func_name" "$@"
  else
      user_choice "Do you want to proceed?" \
          "$func_name" "$@"
  fi
}
