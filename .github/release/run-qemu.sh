#!/usr/bin/env bash
# Boot a released AlkOS ISO in QEMU.
# Usage: ./run-qemu.sh [alkos.iso]   (defaults to the first alkos-*.iso here)
set -e

ISO="${1:-$(ls alkos-*.iso 2>/dev/null | head -n1)}"

qemu-system-x86_64 \
  -cdrom "$ISO" \
  -m 4G -smp 4 \
  -cpu max -machine q35 \
  -serial stdio -no-reboot \
  # -enable-kvm  # Linux: uncomment for KVM acceleration (much faster)
