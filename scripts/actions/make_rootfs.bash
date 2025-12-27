#!/usr/bin/env bash

readonly MAKE_ROOTFS_SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
readonly MAKE_ROOTFS_SCRIPT_PATH="${MAKE_ROOTFS_SCRIPT_DIR}/$(basename "$0")"
readonly MAKE_ROOTFS_IMPL_DIR="${MAKE_ROOTFS_SCRIPT_DIR}/../rootfs"

source "${MAKE_ROOTFS_SCRIPT_DIR}/../utils/helpers.bash"
source "${MAKE_ROOTFS_SCRIPT_DIR}/../utils/conf_handlers.bash"
source "${MAKE_ROOTFS_SCRIPT_DIR}/../utils/argparse.bash"

source_conf_file
verify_conf_var_exists CONF_ROOTFS_DIR
verify_conf_var_exists CONF_ROOTFS_TARGET_PATH
verify_conf_var_exists CONF_ROOTFS_OVERLAY_DIR

# Filesystem types dictionary
declare -A ALK_OS_SUPPORTED_FS=(
    [fat]="FAT filesystem (fat12/fat16/fat32)"
)

help() {
    echo "Examples:"
    echo "    $0 fat --type fat32 --size 200M --label ALKOS"
    echo
}

parse_args() {
    argparse_init "${MAKE_ROOTFS_SCRIPT_PATH}" "Create a rootfs image for AlkOS" help
    argparse_add_option "v|verbose" "Enable verbose output" false false "" "flag"

    local fs_choices=$(echo "${!ALK_OS_SUPPORTED_FS[@]}" | tr ' ' '|')
    argparse_add_positional "filesystem" "Filesystem type" true "$fs_choices"
    argparse_add_variadic "extra_args" "Additional arguments for the chosen filesystem"

    argparse_parse "$@"
}

copy_overlay_files() {
    local overlay_dir="${CONF_ROOTFS_OVERLAY_DIR}"
    local target_dir="${CONF_ROOTFS_DIR}"

    if [[ -d "${overlay_dir}" ]]; then
        cp -rT "${overlay_dir}" "${target_dir}"
    fi
}

copy_userspace_files() {
    local userspace_dir="${CONF_USERSPACE_BIN_DIR}"
    local target_dir="${CONF_ROOTFS_DIR}/bin"

    if [[ -n "${userspace_dir}" && -d "${userspace_dir}" ]]; then
        mkdir -p "${target_dir}"
        cp -v "${userspace_dir}"/* "${target_dir}/" || true
    fi
}

run_delegate() {
    local fs_type="$(argparse_get "filesystem")"
    local fs_script="${MAKE_ROOTFS_IMPL_DIR}/make_${fs_type}.bash"

    # Check if filesystem script exists
    if [[ ! -f "${fs_script}" ]]; then
        dump_error "Filesystem script not found: ${fs_script}"
    fi

    # Execute filesystem-specific script
    base_runner "Failed to make rootfs image for ${fs_type}" "$(argparse_get "v|verbose")" \
    "${fs_script}" "${CONF_ROOTFS_DIR}" "${CONF_ROOTFS_TARGET_PATH}" "$(argparse_get "extra_args")"
}

main() {
    parse_args "$@"
    copy_overlay_files
    copy_userspace_files
    run_delegate
}

main "$@"
