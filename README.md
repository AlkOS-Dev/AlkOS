# AlkOS
[![License](https://img.shields.io/github/license/AlkOS-Dev/AlkOS)](https://github.com/AlkOS-Dev/AlkOS/blob/main/LICENSE)
[![Stars](https://img.shields.io/github/stars/AlkOS-Dev/AlkOS)](https://github.com/AlkOS-Dev/AlkOS/stargazers)
[![Issues](https://img.shields.io/github/issues/AlkOS-Dev/AlkOS)](https://github.com/AlkOS-Dev/AlkOS/issues)
[![Contributors](https://img.shields.io/github/contributors/AlkOS-Dev/AlkOS)](https://github.com/AlkOS-Dev/AlkOS/graphs/contributors)

A modern kernel written in **C++23**. Can run doom.

### ⚡ Capabilities

*   **Architecture:** Portable HAL-based core (x86_64 implemented), Higher Half, SMP-ready.
*   **Memory:** PMM (Buddy/Bitmap), VMM (Recursive mapping, VMA), Heap (Slab).
*   **Scheduling:** Preemptive Multitasking via MLFQ and Round Robin policies.
*   **Userspace:** Ring 3 isolation, ELF64 loader, Syscall interface (`int 0x80`).
*   **Filesystem:** VFS abstraction with FAT12/16/32 and Initrd support.
*   **Graphics:** Linear Framebuffer, Compositor/Window Manager, Double Buffering.
*   **Hardware:** ACPI (via uACPI), IO/Local APIC, HPET, PCI, PS/2, Serial.
*   **Runtime:** Custom `libc` and `libcpp` implementation (no upstream deps).
*   **Apps:** Shell, GUI Demo, **Doom**.

### 🚀 Quick Start

**Prerequisites:** Linux (Arch/Ubuntu recommended) or Docker.

**1. Setup Environment**
Builds the custom GCC 15.1.0 cross-toolchain. System dependency installation is automated for **Arch** and **Ubuntu**.

```bash
./scripts/alkos_cli.bash --install all --verbose
```
> **Note:** On other distributions, install prerequisites manually (see `scripts/env/`), then run with `--install toolchain`.

**2. Configure Build**
Generates CMake configuration and feature flags.
```bash
./scripts/alkos_cli.bash --configure
```

**3. Build & Run**
Compiles the kernel, userspace apps, generates the ISO, and launches QEMU.
```bash
./scripts/alkos_cli.bash --run
```

### 🛠 CLI Tooling

The project is managed via `scripts/alkos_cli.bash`.

| Command | Description |
| :--- | :--- |
| `-i`, `--install [all/deps/toolchain]` | Sets up the dev environment. |
| `-c`, `--configure` | Generates CMake configs and feature flags. |
| `-r`, `--run` | Builds the ISO and boots QEMU. |
| `-g`, `--git-hooks` | Installs pre-commit hooks (clang-format). |

### 📂 Structure

*   `kernel/` - Core kernel source (Arch specific, MM, Sched, Drivers).
*   `libs/` - Custom implementation of `libc`, `libcpp`, and containers.
*   `userspace/` - Ring 3 applications (Shell, Doom, GUI tests).
*   `scripts/` - Build system, CI/CD, and environment automation.

### 🔧 Debugging

To attach GDB to a running QEMU instance:
```bash
# Terminal 1
./scripts/alkos_cli.bash --run --gdb

# Terminal 2
./scripts/install/attach_to_qemu_gdb.bash build/alkos/sysroot/boot/alkos.kernel
```

### 📄 License

MIT License. See [LICENSE](LICENSE).
