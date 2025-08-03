#!/bin/bash

INSTALL_DEPS_ARCH_SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
INSTALL_DEPS_ARCH_SCRIPT_PATH="${INSTALL_DEPS_ARCH_SCRIPT_DIR}/$(basename "$0")"
INSTALL_DEPS_ARCH_SCRIPT_PACKAGES_TXT_FILE="${INSTALL_DEPS_ARCH_SCRIPT_DIR}/arch_packages.txt"

source "${INSTALL_DEPS_ARCH_SCRIPT_DIR}/../utils/pretty_print.bash"
source "${INSTALL_DEPS_ARCH_SCRIPT_DIR}/../utils/helpers.bash"
source "${INSTALL_DEPS_ARCH_SCRIPT_DIR}/../utils/argparse.bash"

parse_args() {
  argparse_init "${INSTALL_DEPS_ARCH_SCRIPT_PATH}" "Install dependencies for Arch-based systems"
  argparse_add_option "i|install" "Install dependencies" true false "" "flag"
  argparse_add_option "a|aur-helper" "Specify AUR helper (default: pacman)" false "pacman" "" "string"
  argparse_add_option "v|verbose" "Enable verbose output" false false "" "flag"
  argparse_parse "$@"
}

process_args() {
  if ! command -v "$(argparse_get "a|aur-helper")" &> /dev/null; then
    dump_error "AUR helper $(argparse_get "a|aur-helper") is not installed"
  fi
}

run_install() {
  pretty_info "Using AUR helper: $(argparse_get "a|aur-helper")"
  pretty_info "Installing dependencies"

  while IFS= read -r package || [ -n "$package" ]; do
    pretty_info "Installing ${package}"
    base_runner "Failed to install ${package}" $(argparse_get "v|verbose")" sudo "$(argparse_get "a|aur-helper")" -S --noconfirm "${package}"
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
  base_runner "Failed to load KVM modules" $(argparse_get "v|verbose")" sudo modprobe "${kvm_module}"
  base_runner "Failed to load KVM CPU module" $(argparse_get "v|verbose")" sudo modprobe "${kvm_cpu_module}"

  # Verify KVM modules are loaded
  if ! lsmod | grep -q "${kvm_module}"; then
    dump_error "Failed to load ${kvm_module}"
  fi

  # Enable KVM modules on boot
  pretty_info "Configuring KVM modules to load on boot"
  if [ ! -f "/etc/modules-load.d/kvm.conf" ]; then
    base_runner "Failed to create KVM modules config" $(argparse_get "v|verbose")" \
      sudo bash -c "echo -e '${kvm_module}\n${kvm_cpu_module}' > /etc/modules-load.d/kvm.conf"
  fi

  pretty_success "KVM setup completed successfully"
}

main() {
  parse_args "$@"
  process_args
  run_install
  enable_kvm
}

main "$@"
