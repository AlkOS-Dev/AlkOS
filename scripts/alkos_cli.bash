#!/bin/bash

# Script location setup
readonly ALK_OS_CLI_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
readonly ALK_OS_CLI_SCRIPT_PATH="${ALK_OS_CLI_SCRIPT_DIR}/$(basename "$0")"


# Script paths
readonly ALK_OS_CLI_INSTALL_TOOLCHAIN_PATH="${ALK_OS_CLI_SCRIPT_DIR}/actions/install_toolchain.bash"
readonly ALK_OS_CLI_BUILD_SCRIPT_PATH="${ALK_OS_CLI_SCRIPT_DIR}/actions/build_alkos.bash"
readonly ALK_OS_CLI_QEMU_RUN_SCRIPT_PATH="${ALK_OS_CLI_SCRIPT_DIR}/actions/run_alkos.bash"
readonly ALK_OS_CLI_CONFIGURE_SCRIPT_PATH="${ALK_OS_CLI_SCRIPT_DIR}/config/configure.bash"
readonly ALK_OS_CLI_CONF_PATH="${ALK_OS_CLI_SCRIPT_DIR}/../config/conf.generated.bash"
readonly ALK_OS_CLI_SETUP_HOOKS_SCRIPT_PATH="${ALK_OS_CLI_SCRIPT_DIR}/git-hooks/setup-hooks.bash"

# Import utilities
source "${ALK_OS_CLI_SCRIPT_DIR}/utils/pretty_print.bash"
source "${ALK_OS_CLI_SCRIPT_DIR}/utils/helpers.bash"
source "${ALK_OS_CLI_SCRIPT_DIR}/utils/argparse.bash"

# Install dependencies scripts dictionary
declare -A ALK_OS_CLI_INSTALL_DEPS_SCRIPT_PATH_DICT=(
    [arch]="env/install_deps_arch.bash"
    [ubuntu]="env/install_deps_ubuntu.bash"
)

print_banner() {
    cat << "EOF"
    _    _ _    _____ _____
   / \  | | | _|  _  |   __|
  / _ \ | | |/ / | | |__   |
 / ___ \| |   <| |_| |     |
/_/   \_\_|_|\_\_____|_____| CLI
EOF
    echo "AlkOS Environment Setup Tool"
    echo "Version: 0.0.0"
    echo
}

parse_args() {
  argparse_init "${ALK_OS_CLI_SCRIPT_PATH}" "AlkOS CLI Tool"
  argparse_add_option "r|run" "Build and run AlkOS" false false "" "flag"
  argparse_add_option "i|install" "Install components" false "" "toolchain|deps|all" "string"
  argparse_add_option "v|verbose" "Enable verbose output" false false "" "flag"
  argparse_add_option "c|configure" "Run default configuration" false false "" "flag"
  argparse_add_option "g|git-hooks" "Setup git hooks" false false "" "flag"
  argparse_parse "$@"
}

validate_args() {
    if [[ $(argparse_get "r|run") == false ]] &&
       [[ $(argparse_get "i|install") == "" ]] &&
       [[ $(argparse_get "c|configure") == false ]] &&
       [[ $(argparse_get "g|git-hooks") == false ]]; then
        dump_error "No action specified. Use --run, --install, --configure or --git-hooks."
        exit 1
    fi
}

run_default_configuration() {
    if [[ $(argparse_get "c|configure") == true ]]; then
        pretty_info "Running default configuration"
        base_runner "Failed to run default configuration" true \
            "${ALK_OS_CLI_CONFIGURE_SCRIPT_PATH}" x86_64 debug $(argparse_get "v|verbose") -p regression_mode
    fi
}

install_dependencies() {
    if [[ $(argparse_get "i|install") == "deps" || $(argparse_get "i|install") == "all" ]]; then
      pretty_info "Installing system dependencies"

      local distro=$(get_supported_distro_name)
      if [[ -z $distro ]]; then
        dump_error "Dependencies installation is not supported on this distribution"
        exit 1
      fi

      local install_script_path="${ALK_OS_CLI_SCRIPT_DIR}/${ALK_OS_CLI_INSTALL_DEPS_SCRIPT_PATH_DICT[$distro]}"
      base_runner "Failed to install dependencies" true \
        "${install_script_path}" --install $(argparse_get "v|verbose")
    fi
}

validate_configuration_exists() {
  if [ ! -f "${ALK_OS_CLI_CONF_PATH}" ] ; then
     dump_error "Coudn't find configuration. Use configure.bash script to configure the environment"
    exit 1
  fi
}

install_toolchain() {
    if [[ $(argparse_get "i|install") == "toolchain" || $(argparse_get "i|install") == "all" ]]; then
        validate_configuration_exists

        pretty_info "Installing cross-compile toolchain for ${ALK_OS_CLI_CONFIG[arch]}"
        base_runner "Failed to install cross-compile toolchain" true \
            "${ALK_OS_CLI_INSTALL_TOOLCHAIN_PATH}" $(argparse_get "v|verbose")
    fi
}

build_and_run() {
    if [[ $(argparse_get "r|run")} == true ]]; then
        validate_configuration_exists

        pretty_info "Building AlkOS..."
        base_runner "Failed to build AlkOS" true "${ALK_OS_CLI_BUILD_SCRIPT_PATH}" $(argparse_get "v|verbose")

        pretty_info "Running AlkOS in QEMU"
        base_runner "Failed to run AlkOS in QEMU" true "${ALK_OS_CLI_QEMU_RUN_SCRIPT_PATH}" --verbose
    fi
}

setup_git_hooks() {
  if [[ $(argparse_get "g|git-hooks") == true ]]; then
    base_runner "Failed to setup git-hooks" true "${ALK_OS_CLI_SETUP_HOOKS_SCRIPT_PATH}"
  fi
}

main() {
    print_banner
    parse_args "$@"
    validate_args
    install_dependencies
    run_default_configuration
    install_toolchain
    build_and_run
    setup_git_hooks
}

main "$@"
