#!/usr/bin/env bash
# Boot a released AlkOS ISO in QEMU.
# Usage: ./run-qemu.sh [alkos.iso]   (defaults to the first alkos-*.iso here)
# On Linux, add for speed:  -enable-kvm -cpu max,+invtsc -machine accel=kvm
set -e

ISO="${1:-$(ls alkos-*.iso 2>/dev/null | head -n1)}"

qemu-system-x86_64 \
  -cdrom "$ISO" \
  -m 4G -smp 4 \
  -cpu max -machine q35 \
  -serial stdio -no-reboot
