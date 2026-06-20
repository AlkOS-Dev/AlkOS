# Running AlkOS

How to boot a released AlkOS ISO. To build from source instead, see the
[AlkOS README](https://github.com/AlkOS-Dev/AlkOS).

## Requirements

- x86_64 host with [QEMU](https://www.qemu.org/) (`qemu-system-x86_64`).
- ~4 GiB RAM for the guest.
- Linux + KVM is recommended for speed, without it QEMU still works, just slower.

## Quick start

```bash
chmod +x run-qemu.sh
./run-qemu.sh alkos-x86_64-<version>.iso
```

## Manual QEMU command

```bash
# Portable (works anywhere, slower):
qemu-system-x86_64 -cdrom alkos-x86_64-<version>.iso \
  -m 4G -smp 4 -cpu max -machine q35 -serial stdio -no-reboot

# Linux with KVM (fast): add
#   -enable-kvm -cpu max,+invtsc,migratable=off -machine type=q35,accel=kvm
```

`-serial stdio` shows kernel logs in your terminal, the shell and DOOM appear in
the QEMU window.

## Verify your download

```bash
sha256sum -c SHA256SUMS
```

## License

AlkOS is MIT-licensed (see `LICENSE`). The image also bundles third-party
components under their own licenses.
