# Changelog

All notable changes to AlkOS are documented in this file.

## [Unreleased]

## [0.1.0] - 2026-06-20

First release of AlkOS - a hobby operating system built from the ground up
for x86_64, booting into a monolithic higher-half kernel with its own C library and
a userspace real enough to run DOOM.

### Added
- 64-bit higher-half kernel for x86_64.
- Physical, virtual, and heap memory management.
- Preemptive multitasking and scheduling.
- Ring 3 userspace with syscalls and ELF64 loading.
- Virtual filesystem with FAT support.
- Framebuffer graphics with a window manager.
- ACPI and PCI device support.
- In-tree C library (libc) and C++ runtime/utilities (libcpp) instead of upstream ones.
- Userspace applications, including DOOM.
- Build tooling: dedicated GCC cross-toolchain, CMake build, and the `alkos_cli.bash` driver.
- Configurable feature flags and host/kernel test suites.

[Unreleased]: https://github.com/AlkOS-Dev/AlkOS/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/AlkOS-Dev/AlkOS/releases/tag/v0.1.0
