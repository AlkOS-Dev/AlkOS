#!/usr/bin/env bash

readonly MAKE_FAT_IMAGE_SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

source "${MAKE_FAT_IMAGE_SCRIPT_DIR}/../utils/helpers.bash"
source "${MAKE_FAT_IMAGE_SCRIPT_DIR}/../utils/argparse.bash"

# FAT filesystem default configurations
declare -A MAKE_FAT_DEFAULT_CONFIG=(
    [type]="fat16"
    [size]="16M"
    [label]="ALKOS"
    [sector_size]=512
    [sectors_per_cluster]=2
)

parse_args() {
    argparse_init "$(basename "$0")" "Create a FAT filesystem image"

    argparse_add_option "type" "FAT type (fat12, fat16, fat32)" false "${MAKE_FAT_DEFAULT_CONFIG[type]}" "fat12|fat16|fat32"
    argparse_add_option "size" "Size of the image" false "${MAKE_FAT_DEFAULT_CONFIG[size]}"
    argparse_add_option "label" "Volume label" false "${MAKE_FAT_DEFAULT_CONFIG[label]}"
    argparse_add_option "sector-size" "Sector size in bytes" false "${MAKE_FAT_DEFAULT_CONFIG[sector_size]}"
    argparse_add_option "sectors-per-cluster" "Sectors per cluster" false "${MAKE_FAT_DEFAULT_CONFIG[sectors_per_cluster]}"

    argparse_add_positional "source_dir" "Source directory to copy from" true
    argparse_add_positional "target_dir" "Target image file path" true

    argparse_parse "$@"
}

make_fat_image() {
    local source_dir="$(argparse_get "source_dir")"
    local target_dir="$(argparse_get "target_dir")"
    local fat_type="$(argparse_get "type")"
    local size="$(argparse_get "size")"
    local label="$(argparse_get "label")"
    local sector_size="$(argparse_get "sector-size")"
    local sectors_per_cluster="$(argparse_get "sectors-per-cluster")"

    # Get size in bytes and round up to the nearest 512-byte block
    local size_bytes=$(convert_to_bytes "$size")
    local block_count=$(ceil_division "$size_bytes" 512)
    local target_size_bytes=$(("$block_count" * 512))

    pretty_info "Creating FAT rootfs image with size $(from_bytes_to_human_readable "$target_size_bytes")..."

    dd if=/dev/zero of="$target_dir" \
       bs=512 count="$block_count" status=progress

    mkfs.fat "${target_dir}" \
             -F "${fat_type/fat/}" \
             -n "${label}" \
             -s "${sectors_per_cluster}" \
             -S "${sector_size}"

    pretty_info "FAT rootfs image created successfully."

    if ! command -v mcopy &>/dev/null; then
        dump_error "mtools is not installed. Please install it to use this script."
        exit 1
    fi

    pretty_info "Copy the source directory to the image..."
    mcopy -i "${target_dir}" -sv "${source_dir}"/* ::/
}

main() {
    parse_args "$@"
    make_fat_image
}

main "$@"
