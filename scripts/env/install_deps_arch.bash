#!/bin/bash

INSTALL_DEPS_ARCH_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
INSTALL_DEPS_ARCH_SCRIPT_PATH="${INSTALL_DEPS_ARCH_SCRIPT_DIR}/$(basename "$0")"
INSTALL_DEPS_ARCH_SCRIPT_PACKAGES_TXT_FILE="${INSTALL_DEPS_ARCH_SCRIPT_DIR}/arch_packages.txt"

source "${INSTALL_DEPS_ARCH_SCRIPT_DIR}/../utils/pretty_print.bash"
source "${INSTALL_DEPS_ARCH_SCRIPT_DIR}/../utils/helpers.bash"

help() {
  echo "${INSTALL_DEPS_ARCH_SCRIPT_PATH} --install [--verbose | -v] [--aur-helper [yay | paru | ...] | -a]"
  echo "Where:"
  echo "--install | -i - required flag to start installation"
  echo "--verbose | -v - flag to enable verbose output"
  echo "--aur-helper [yay | paru | ...] | -a - flag to specify and use an AUR helper (uses pacman by default)"
  echo "Note: this script is intended to be run on Arch-based systems"
}

run_install() {
  pretty_info "Using AUR helper: ${AUR_HELPER}"
  pretty_info "Installing dependencies"

  while IFS= read -r package || [ -n "$package" ]; do
    pretty_info "Installing ${package}"
    base_runner "Failed to install ${package}" "${VERBOSE}" sudo "${AUR_HELPER}" -S --noconfirm "${package}"
    pretty_success "Correctly installed: ${package}"
  done < "${INSTALL_DEPS_ARCH_SCRIPT_PACKAGES_TXT_FILE}"

  pretty_success "Dependencies installed"
}

enable_kvm() {
  pretty_info "Enabling KVM..."

  # Check if virtualization is supported by hardware
  if ! grep -E 'vmx|svm' /proc/cpuinfo &> /dev/null; then
    dump_error "Virtualization is not supported by your CPU or is disabled in BIOS"
  fi

  local cpu_vendor
  local kvm_module="kvm"
  local kvm_cpu_module

  cpu_vendor=$(grep -E '^vendor_id' /proc/cpuinfo | head -n 1 | awk '{print $3}')
  case "$cpu_vendor" in
    GenuineIntel)
      kvm_cpu_module="kvm_intel"
      ;;
    AuthenticAMD)
      kvm_cpu_module="kvm_amd"
      ;;
    *)
      dump_error "Unsupported CPU vendor: ${cpu_vendor}"
      ;;
  esac

  # Check if KVM modules are already loaded
  if lsmod | grep -q "${kvm_module}" && lsmod | grep -q "${kvm_cpu_module}"; then
    pretty_success "KVM modules already loaded. Skipping setup..."
    return 0
  fi

  # Load KVM modules
  pretty_info "Loading KVM modules for ${cpu_vendor} CPU"
  base_runner "Failed to load KVM modules" "${VERBOSE}" sudo modprobe "${kvm_module}"
  base_runner "Failed to load KVM CPU module" "${VERBOSE}" sudo modprobe "${kvm_cpu_module}"

  # Verify KVM modules are loaded
  if ! lsmod | grep -q "${kvm_module}"; then
    dump_error "Failed to load ${kvm_module}"
  fi

  # Enable KVM modules on boot
  pretty_info "Configuring KVM modules to load on boot"
  if [ ! -f "/etc/modules-load.d/kvm.conf" ]; then
    base_runner "Failed to create KVM modules config" "${VERBOSE}" \
      sudo bash -c "echo -e '${kvm_module}\n${kvm_cpu_module}' > /etc/modules-load.d/kvm.conf"
  fi

  pretty_success "KVM setup completed successfully"
}

parse_args() {
  INSTALL_FOUND=false
  VERBOSE=false
  AUR_HELPER="pacman"

  while [[ $# -gt 0 ]]; do
    case $1 in
      -h|--help)
        help
        exit 0
        ;;
      -i|--install)
        INSTALL_FOUND=true
        shift
        ;;
      -a|--aur-helper)
        AUR_HELPER="$2"
        shift
        shift
        ;;
      -v|--verbose)
        VERBOSE=true
        shift
        ;;
      *)
        echo "Unknown argument: $1"
        exit 1
        ;;
    esac
  done
}

process_args() {
  if [ "$INSTALL_FOUND" = false ]; then
    dump_error "--install flag was not provided!"
  fi

  if ! command -v "${AUR_HELPER}" &> /dev/null; then
    dump_error "AUR helper ${AUR_HELPER} is not installed"
  fi
}

main() {
  parse_args "$@"
  process_args
  run_install
  enable_kvm
}

main "$@"
