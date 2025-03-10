#!/usr/bin/env bash

CROSS_COMPILE_BUILD_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
CROSS_COMPILE_BUILD_SCRIPT_PATH="${CROSS_COMPILE_BUILD_SCRIPT_DIR}/$(basename "$0")"

CROSS_COMPILE_BUILD_BIN_UTILS_VER=""
CROSS_COMPILE_BUILD_GDB_VER=""
CROSS_COMPILE_BUILD_GCC_VER=""
CROSS_COMPILE_BUILD_TARGET=x86_64-elf

CROSS_COMPILE_BUILD_POSITIONAL_ARGS=()
CROSS_COMPILE_BUILD_INSTALL_FOUND=false
CROSS_COMPILE_BUILD_VERBOSE=false
CROSS_COMPILE_USES_DEFAULT_TARGET=true

PROC_COUNT=$(nproc --all)

source "${CROSS_COMPILE_BUILD_SCRIPT_DIR}/../utils/pretty_print.bash"
source "${CROSS_COMPILE_BUILD_SCRIPT_DIR}/../utils/helpers.bash"

help() {
    echo "${CROSS_COMPILE_BUILD_SCRIPT_PATH} --install [--build_dir | -b <dir>] [--tool_dir | -t <dir>] [--custom_target | -c <target>]"
    echo "Where:"
    echo "--install         | -i - required flag to start installation"
    echo "--build_dir <dir> | -b <dir> - provides directory <dir> to save all build files"
    echo "--tool_dir  <dir> | -t <dir> - directory where tooling should be saved"
    echo "--custom_target   | -c - custom target to build cross-compiler for (default: x86_64-elf)"
    echo "--verbose         | -v - flag to enable verbose output"
}

runner() {
    assert_argument_provided "$1"

    local dump_info="$1"
    shift

    base_runner "${dump_info}" "${CROSS_COMPILE_BUILD_VERBOSE}" "$@"
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
    local link="$2"

    if ! [ -f "${name}" ]; then
        pretty_info "Downloading ${name}"
        runner "Failed to download ${name}" wget --no-verbose --show-progress --progress=bar:force:noscroll "${link}"
        pretty_success "${name} downloaded correctly"
    else
        pretty_info "Skipping... ${name} already downloaded"
    fi
}

download_extract_gnu_source() {
    assert_argument_provided "$1"
    assert_argument_provided "$2"
    local name="$1"
    local link="$2"

    download_source "${name}" "${link}/${name}"

    pretty_info "Extracting ${name}"
    runner "Failed to extract ${name}" tar -xf "${name}"
    pretty_success "${name} extracted correctly"
}

build_binutils() {
    local binutils_dir="${CROSS_COMPILE_BUILD_BUILD_DIR}/build_binutils"
    local binutils_name="binutils-${CROSS_COMPILE_BUILD_BIN_UTILS_VER}"

    pretty_info "Building binutils"

    prepare_directory "${binutils_dir}"
    cd "${binutils_dir}"

    download_extract_gnu_source "${binutils_name}.tar.gz" "https://ftp.gnu.org/gnu/binutils"

    pretty_info "Configuring binutils"
    runner "Failed to configure binutils" ${binutils_name}/configure --target="${CROSS_COMPILE_BUILD_TARGET}" --prefix="${CROSS_COMPILE_BUILD_TOOL_DIR}" --with-sysroot --disable-nls --disable-werror
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

    attempt_runner "Libgcc build failed due to PIC issues" \
        make -j "${PROC_COUNT}" all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 -mcmodel=kernel -mno-red-zone'
    if [ $? -ne 0 ]; then
        pretty_info "Libgcc build failed due to PIC issues; applying sed fix."
        sed -i 's/PICFLAG/DISABLED_PICFLAG/g' "${CROSS_COMPILE_BUILD_TARGET}/libgcc/Makefile"
        pretty_info "Retrying libgcc build after sed fix."
        attempt_runner "Failed to build libgcc even after applying the sed fix" \
            make -j "${PROC_COUNT}" all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 -mcmodel=kernel -mno-red-zone'
    fi
    pretty_success "libgcc built correctly"
}

build_gcc() {
    local gcc_dir="${CROSS_COMPILE_BUILD_BUILD_DIR}/build_gcc"
    local gcc_name="gcc-${CROSS_COMPILE_BUILD_GCC_VER}"

    pretty_info "Building GCC"

    prepare_directory "${gcc_dir}"
    cd "${gcc_dir}"

    download_extract_gnu_source "${gcc_name}.tar.gz" "https://ftp.gnu.org/gnu/gcc/${gcc_name}"

    pretty_info "Configuring GCC"
    runner "Failed to configure GCC" ${gcc_name}/configure --target="${CROSS_COMPILE_BUILD_TARGET}" --prefix="${CROSS_COMPILE_BUILD_TOOL_DIR}" --disable-nls --enable-languages=c,c++ --without-headers
    pretty_success "GCC configured correctly"

    pretty_info "Building GCC with ${PROC_COUNT} threads"
    runner "Failed to build GCC" make -j "${PROC_COUNT}" all-gcc
    pretty_success "GCC built correctly"

    pretty_info "Building libgcc"
    if [ "$CROSS_COMPILE_BUILD_TARGET" = "x86_64-elf" ]; then
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
    local gdb_dir="${CROSS_COMPILE_BUILD_BUILD_DIR}/build_gdb"
    local gdb_name="gdb-${CROSS_COMPILE_BUILD_GDB_VER}"

    pretty_info "Building GDB"

    prepare_directory "${gdb_dir}"
    cd "${gdb_dir}"

    download_extract_gnu_source "${gdb_name}.tar.gz" "https://ftp.gnu.org/gnu/gdb/"

    pretty_info "Configuring GDB"
    runner "Failed to configure GDB" ${gdb_name}/configure --target=${CROSS_COMPILE_BUILD_TARGET} --prefix="${CROSS_COMPILE_BUILD_TOOL_DIR}" --disable-werror
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
    pretty_info "Starting GCC cross-compiler build with build directory: ${CROSS_COMPILE_BUILD_BUILD_DIR} and target directory: ${CROSS_COMPILE_BUILD_TOOL_DIR}"

    check_is_in_env_path "${CROSS_COMPILE_BUILD_TOOL_DIR}/bin"
    local is_in_path=$?

    export PATH="${CROSS_COMPILE_BUILD_TOOL_DIR}/bin:${PATH}"
    export PREFIX="${CROSS_COMPILE_BUILD_TOOL_DIR}"

    build_binutils
    build_gdb
    build_gcc

    add_to_user_env_path "${CROSS_COMPILE_BUILD_TOOL_DIR}/bin" "${is_in_path}"

    pretty_success "Build: ${CROSS_COMPILE_BUILD_SCRIPT_PATH} completed successfully"
}

parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -t|--tool_dir)
                CROSS_COMPILE_BUILD_TOOL_DIR="$2"
                shift
                shift
                ;;
            -b|--build_dir)
                CROSS_COMPILE_BUILD_BUILD_DIR="$2"
                shift
                shift
                ;;
            -h|--help)
                help
                exit 0
                ;;
            -i|--install)
                CROSS_COMPILE_BUILD_INSTALL_FOUND=true
                shift
                ;;
            -v|--verbose)
                CROSS_COMPILE_BUILD_VERBOSE=true
                shift
                ;;
            -c|--custom_target)
                CROSS_COMPILE_BUILD_TARGET="$2"
                CROSS_COMPILE_USES_DEFAULT_TARGET=false
                shift
                shift
                ;;
            -*)
                dump_error "Unknown option $1"
                ;;
            *)
                CROSS_COMPILE_BUILD_POSITIONAL_ARGS+=("$1")
                shift
                ;;
        esac
    done

    set -- "${CROSS_COMPILE_BUILD_POSITIONAL_ARGS[@]}"
}

process_args() {
    if [ $CROSS_COMPILE_BUILD_INSTALL_FOUND = false ] ; then
        dump_error "--install flag was not provided!"
    fi

    if [ -z "${CROSS_COMPILE_BUILD_TOOL_DIR}" ] ; then
        dump_error "--tool_dir | -t flag with directory where save tooling was not provided!"
    fi

    if [ -z "${CROSS_COMPILE_BUILD_BUILD_DIR}" ] ; then
        dump_error "--build_dir | -t flag with directory where save build files was not provided!"
    fi

    if [ $CROSS_COMPILE_USES_DEFAULT_TARGET = false ] ; then
        pretty_info "Using custom target: ${CROSS_COMPILE_BUILD_TARGET}"
    else
        pretty_info "Using default target: ${CROSS_COMPILE_BUILD_TARGET}"
    fi
}

main() {
    parse_args "$@"
    process_args
    load_toolchain_versions
    run_build
}

main "$@"
