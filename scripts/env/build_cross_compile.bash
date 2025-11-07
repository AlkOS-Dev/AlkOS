#!/usr/bin/env bash

CROSS_COMPILE_BUILD_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
CROSS_COMPILE_BUILD_SCRIPT_PATH="${CROSS_COMPILE_BUILD_SCRIPT_DIR}/$(basename "$0")"

CROSS_COMPILE_BUILD_BIN_UTILS_VER=""
CROSS_COMPILE_BUILD_GDB_VER=""
CROSS_COMPILE_BUILD_GCC_VER=""

PROC_COUNT=$(($(nproc) + 1))

source "${CROSS_COMPILE_BUILD_SCRIPT_DIR}/../utils/pretty_print.bash"
source "${CROSS_COMPILE_BUILD_SCRIPT_DIR}/../utils/helpers.bash"
source "${CROSS_COMPILE_BUILD_SCRIPT_DIR}/../utils/argparse.bash"

parse_args() {
    argparse_init "${CROSS_COMPILE_BUILD_SCRIPT_PATH}" "Build cross-compiler toolchain"
    argparse_add_option "i|install" "Install the cross-compiler toolchain" true false "" "flag"
    argparse_add_option "b|build_dir" "Directory to save build files" true "" "" "directory"
    argparse_add_option "t|tool_dir" "Directory to save toolchain files" true "" "" "directory"
    argparse_add_option "c|custom_target" "Custom target to build cross-compiler for" false "x86_64-elf" "x86_64-elf|i386-elf" "string"
    argparse_add_option "v|verbose" "Enable verbose output" false false "" "flag"
    argparse_parse "$@"
}

runner() {
    assert_argument_provided "$1"

    local dump_info="$1"
    shift

    base_runner "${dump_info}" "$(argparse_get "v|verbose")" "$@"
}

load_toolchain_versions() {
    pretty_info "Loading toolchain versions"
    local toolchain_versions_file="${CROSS_COMPILE_BUILD_SCRIPT_DIR}/toolchain_versions.txt"

    if ! [ -f "${toolchain_versions_file}" ]; then
        pretty_error "Toolchain versions file not found: ${toolchain_versions_file}"
        exit 1
    fi

    while IFS='=' read -r key value; do
        case "${key,,}" in
            binutils)
                CROSS_COMPILE_BUILD_BIN_UTILS_VER="${value}"
                ;;
            gcc)
                CROSS_COMPILE_BUILD_GCC_VER="${value}"
                ;;
            gdb)
                CROSS_COMPILE_BUILD_GDB_VER="${value}"
                ;;
            *)
                pretty_error "Unknown toolchain version: ${key}-${value}"
                exit 1
                ;;
        esac
    done < "${toolchain_versions_file}"

    if [ -z "${CROSS_COMPILE_BUILD_BIN_UTILS_VER}" ]; then
        pretty_error "Binutils version not found"
        exit 1
    fi

    if [ -z "${CROSS_COMPILE_BUILD_GCC_VER}" ]; then
        pretty_error "GCC version not found"
        exit 1
    fi

    if [ -z "${CROSS_COMPILE_BUILD_GDB_VER}" ]; then
        pretty_error "GDB version not found"
        exit 1
    fi

    pretty_success "Toolchain versions loaded correctly"
    pretty_info "Binutils version: ${CROSS_COMPILE_BUILD_BIN_UTILS_VER}"
    pretty_info "GCC version: ${CROSS_COMPILE_BUILD_GCC_VER}"
    pretty_info "GDB version: ${CROSS_COMPILE_BUILD_GDB_VER}"
}

prepare_directory() {
    assert_argument_provided "$1"
    local dir="$1"

    if ! [ -d "${dir}" ]; then
        pretty_info "Creating directory ${dir}"
        mkdir -p "${dir}" || dump_error "Failed to create directory ${dir}"
    else
        pretty_info "Directory ${dir} already exists"
    fi
}

download_source() {
    assert_argument_provided "$1"
    assert_argument_provided "$2"
    local name="$1"
    local primary_link="$2"
    local fallback_link="${3:-}"

    if ! [ -f "${name}" ]; then
        pretty_info "Downloading ${name}"

        # Try primary link first
        attempt_runner "Failed to download ${name} from primary mirror" "$(argparse_get "v|verbose")" \
            wget --no-verbose --show-progress --progress=bar:force:noscroll "${primary_link}"
        if [ $? -eq 0 ]; then
            pretty_success "${name} downloaded correctly from primary mirror"
        elif [ -n "${fallback_link}" ]; then
            # If primary fails and fallback exists, try fallback
            pretty_info "Primary download failed, trying fallback mirror"
            attempt_runner "Failed to download ${name} from fallback mirror" "$(argparse_get "v|verbose")" \
                wget --no-verbose --show-progress --progress=bar:force:noscroll "${fallback_link}"
            if [ $? -eq 0 ]; then
                pretty_success "${name} downloaded correctly from fallback mirror"
            else
                pretty_error "Failed to download ${name} from both mirrors"
                exit 1
            fi
        else
            # No fallback provided
            pretty_error "Failed to download ${name} and no fallback provided"
            exit 1
        fi
    else
        pretty_info "Skipping... ${name} already downloaded"
    fi
}

download_extract_gnu_source() {
    assert_argument_provided "$1"
    assert_argument_provided "$2"
    local name="$1"
    local primary_mirror="$2"
    local fallback_mirror="${3:-https://ftp.gnu.org/gnu}"
    local filename=$(basename "${name}")

    download_source "${filename}" "${primary_mirror}/${name}" "${fallback_mirror}/${name}"

    pretty_info "Extracting ${filename}"
    runner "Failed to extract ${filename}" tar -xf "${filename}"
    pretty_success "${filename} extracted correctly"
}

build_binutils() {
    local sysroot_path="$1"
    local binutils_dir="$(argparse_get "b|build_dir")/build_binutils"
    local binutils_name="binutils-${CROSS_COMPILE_BUILD_BIN_UTILS_VER}"

    pretty_info "Building binutils"

    prepare_directory "${binutils_dir}"
    cd "${binutils_dir}"

    download_extract_gnu_source "binutils/releases/${binutils_name}.tar.gz" "https://sourceware.org/pub/"

    pretty_info "Configuring binutils"
    runner "Failed to configure binutils" ${binutils_name}/configure --target="$(argparse_get "c|custom_target")" --prefix="$(argparse_get "t|tool_dir")" --with-sysroot="${sysroot_path}" --disable-nls --disable-werror
    pretty_success "Binutils configured correctly"

    pretty_info "Building binutils with ${PROC_COUNT} threads"
    runner "Failed to build binutils" make -j "${PROC_COUNT}"
    pretty_success "Binutils built correctly"

    pretty_info "Installing binutils"
    runner "Failed to install binutils" make install
    pretty_success "Binutils installed correctly"

    pretty_success "Binutils build completed"
}

build_libgcc_with_retry_x86_64_fix() {
    # https://wiki.osdev.org/Building_libgcc_for_mcmodel%3Dkernel
    pretty_info "Building libgcc with retry logic"

    attempt_runner "Libgcc build failed due to PIC issues" "$(argparse_get "v|verbose")" \
        make -j "${PROC_COUNT}" all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 -mcmodel=kernel -mno-red-zone'
    if [ $? -ne 0 ]; then
        pretty_info "Libgcc build failed due to PIC issues; applying sed fix."
        sed -i 's/PICFLAG/DISABLED_PICFLAG/g' "$(argparse_get "c|custom_target")/libgcc/Makefile"
        pretty_info "Retrying libgcc build after sed fix."
        attempt_runner "Failed to build libgcc even after applying the sed fix" "$(argparse_get "v|verbose")" \
            make -j "${PROC_COUNT}" all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 -mcmodel=kernel -mno-red-zone'
    fi
    pretty_success "libgcc built correctly"
}

build_gcc() {
    local sysroot_path="$1"
    local gcc_dir="$(argparse_get "b|build_dir")/build_gcc"
    local gcc_name="gcc-${CROSS_COMPILE_BUILD_GCC_VER}"

    pretty_info "Building GCC"

    prepare_directory "${gcc_dir}"
    cd "${gcc_dir}"

    download_extract_gnu_source "gcc/releases/${gcc_name}/${gcc_name}.tar.gz" "https://sourceware.org/pub/"

    pretty_info "Configuring GCC"
    runner "Failed to configure GCC" ${gcc_name}/configure --target="$(argparse_get "c|custom_target")" --prefix="$(argparse_get "t|tool_dir")" --disable-nls --enable-languages=c,c++ --without-headers --with-sysroot="${sysroot_path}" --enable-multilib
    pretty_success "GCC configured correctly"

    pretty_info "Building GCC with ${PROC_COUNT} threads"
    runner "Failed to build GCC" make -j "${PROC_COUNT}" all-gcc
    pretty_success "GCC built correctly"

    pretty_info "Building libgcc"
    if [ "$(argparse_get "c|custom_target")" = "x86_64-elf" ]; then
            build_libgcc_with_retry_x86_64_fix
    else
        runner "Failed to build libgcc" make -j "${PROC_COUNT}" all-target-libgcc CFLAGS_FOR_TARGET='-mno-red-zone'
    fi
    pretty_success "libgcc built correctly"

    pretty_info "Installing GCC"
    runner "Failed to install GCC" make install-gcc
    pretty_success "GCC installed correctly"

    pretty_info "Installing libgcc"
    runner "Failed to install libgcc" make install-target-libgcc
    pretty_success "libgcc installed correctly"

    pretty_success "GCC build completed"
}

build_gdb() {
    local gdb_dir="$(argparse_get "b|build_dir")/build_gdb"
    local gdb_name="gdb-${CROSS_COMPILE_BUILD_GDB_VER}"

    pretty_info "Building GDB"

    prepare_directory "${gdb_dir}"
    cd "${gdb_dir}"

    download_extract_gnu_source "gdb/releases/${gdb_name}.tar.gz" "https://sourceware.org/pub/"

    pretty_info "Configuring GDB"
    runner "Failed to configure GDB" ${gdb_name}/configure --target=$(argparse_get "c|custom_target") --prefix="$(argparse_get "t|tool_dir")" --disable-werror
    pretty_success "GDB configured correctly"

    pretty_info "Building GDB with ${PROC_COUNT} threads"
    runner "Failed to build GDB" make -j "${PROC_COUNT}" all-gdb
    pretty_success "GDB built correctly"

    pretty_info "Installing GDB"
    runner "Failed to install GDB" make install-gdb
    pretty_success "GDB installed correctly"

    pretty_success "GDB build completed"
}

run_build() {
    pretty_info "Starting GCC cross-compiler build with build directory: $(argparse_get "b|build_dir") and target directory: $(argparse_get "t|tool_dir")"

    local sysroot_path="$(argparse_get "t|tool_dir")/$(argparse_get "c|custom_target")/sysroot"
    check_is_in_env_path "$(argparse_get "t|tool_dir")/bin"
    local is_in_path=$?

    export PATH="$(argparse_get "t|tool_dir")/bin:${PATH}"
    export PREFIX="$(argparse_get "t|tool_dir")"

    build_binutils "${sysroot_path}"
    build_gdb
    build_gcc "${sysroot_path}"

    add_to_user_env_path "$(argparse_get "t|tool_dir")/bin" "${is_in_path}"

    pretty_success "Build: ${CROSS_COMPILE_BUILD_SCRIPT_PATH} completed successfully"
}

main() {
    parse_args "$@"
    load_toolchain_versions
    run_build
}

main "$@"
