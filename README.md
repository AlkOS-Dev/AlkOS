# AlkOS

[![License](https://img.shields.io/github/license/Jlisowskyy/AlkOS)](https://github.com/Jlisowskyy/AlkOS/blob/main/LICENSE)
[![Stars](https://img.shields.io/github/stars/Jlisowskyy/AlkOS)](https://github.com/Jlisowskyy/AlkOS/stargazers)
[![Issues](https://img.shields.io/github/issues/Jlisowskyy/AlkOS)](https://github.com/Jlisowskyy/AlkOS/issues)
[![Contributors](https://img.shields.io/github/contributors/Jlisowskyy/AlkOS)](https://github.com/Jlisowskyy/AlkOS/graphs/contributors)

AlkOS is an open-source operating system project targeting the x86_64 architecture. This project serves as an engineering thesis, focusing on the development of a functional operating system kernel. The immediate goal is to implement a preemptive multitasking scheduler. Future plans include developing a shell and support for user-mode programs.

**Current Features:**

*   **Boots to a 64-bit kernel:** The system successfully loads and executes a 64-bit kernel.
*   **Basic memory management:**  Includes setting up page tables for identity mapping the first 1 GiB of memory using 2 MiB pages in the boot process.
*   **Serial port debugging output:**  Supports sending debug messages to a serial port, which is captured by QEMU and can be redirected to the standard output or a file.
*   **Custom testing framework:** Similar to Google Test, allowing for easy definition and execution of test cases within the kernel environment. Tests can be filtered and managed using command-line arguments. The framework supports test assertions (e.g., `ASSERT`, `R_ASSERT_EQ`) and automatic test discovery.
*   **Support for floating-point operations:** The kernel correctly sets up the x87 FPU and AVX extensions, allowing for floating-point calculations.
*   **Interrupt handling:** The kernel includes an Interrupt Descriptor Table (IDT) and can handle basic interrupts. Programmable Interrupt Controller (PIC) is configured to handle hardware interrupts.
*   **Support for GCC C++ Standard Library features:** Includes features like new/delete operators and static initializers.
*   **Stack Smashing Protection:** The kernel utilizes the `-fstack-protector-all` flag during compilation and has a basic implementation of stack canaries to detect stack buffer overflows.
*   **VGA driver:** Supports basic text output to the screen using the VGA text mode.
*   **C Standard Library:** A partial implementation of the C standard library is provided, including functions like `snprintf`, `strlen`, `strcpy`, `strcmp`, `memcpy`, `memset`, etc.
*   **Input from serial port:** The kernel can read input from the serial port, enabling basic interaction via a terminal emulator.

## Table of Contents

*   [AlkOS](#alkos)
    *   [Project Goals](#project-goals)
    *   [Directory Structure](#-directory-structure)
    *   [Key Scripts](#️-key-scripts)
    *   [Getting Started](#getting-started)
        *   [Prerequisites](#-prerequisites)
        *   [Installation](#️-installation-dependencies-and-toolchain)
    *   [Development Workflow](#-development-workflow)
        *   [Building AlkOS](#️-building-alkos)
        *   [Running AlkOS in QEMU](#️-running-alkos-in-qemu)
        *   [Running Tests](#-running-tests)
        *   [Debugging with GDB](#-debugging-with-gdb)
        *   [Applying clang-format for Code Style](#-applying-clang-format-for-code-style)
    *   [License](#-license)

## Project Goals

The primary goals of AlkOS are:

1.  **Implement a preemptive multitasking scheduler:** This is the current focus and a critical step towards a functional operating system.
2.  **Develop a shell:** A basic shell will allow user interaction with the system.
3.  **Support user-mode programs:** This will enable the execution of user applications.
4.  **Serve as an engineering thesis:** AlkOS is being developed as part of an academic project to demonstrate OS development principles.

## 📁 Directory Structure

*   **alkos/** - Contains the source code for the operating system.
    *   **kernel/** - The core kernel code.
        *   **abi/** - Definitions of the kernel ABI (Application Binary Interface).
        *   **arch/** - Architecture-specific code.
            *   **x86_64/** - Code specific to the x86_64 architecture.
                *   **boot32/** - 32-bit bootloader code.
                *   **boot64/** - 64-bit kernel code.
                *   **common/** - Architecture-specific code shared between bootloaders and the kernel.
        *   **include/** - Public kernel header files.
        *   **src/** - Kernel source files.
        *   **test/** - Test framework and test cases for the kernel.
    *   **libc/** - Implementation of the C standard library.
        *   **arch/** - Architecture-specific code for the C standard library.
        *   **cxx/** - C++ support code (e.g., new/delete, static initializers).
        *   **include/** - Public header files for the C standard library.
        *   **io/** - I/O related functions (e.g., `snprintf`).
        *   **libssp/** - Stack Smashing Protector library.
        *   **math/** - Math functions.
        *   **memory/** - Memory manipulation functions.
        *   **string/** - String manipulation functions.
    *   **toolchains/** - CMake toolchain files for cross-compilation.
*   **scripts/**
    *   **actions/** - Wrappers for other scripts, that perform useful operations on the repository (e.g., formatting, cleaning, building, running).
    *   **env/** - Scripts for preparing, updating, and modifying the development and user environments.
    *   **install/** - Scripts for performing user operations like preparing ISO, compiling user versions, or installing the OS.
    *   **tests/** - Scripts and framework used to automatically test the project.
    *   **utils/** - Utility scripts to be sourced and used by other scripts.
    *   **git-hooks/** - Git hooks for the project.
    *   `alkos_cli.bash` - Main CLI script for managing the project.
    *   `conf.bash` - Configuration file.
    *   `configure.bash` - Configuration script.

## 🛠️ Key Scripts

*   `scripts/alkos_cli.bash` - Main CLI script that orchestrates the project. It can install dependencies, build the project, run it in QEMU, and more.
*   `scripts/configure.bash` - Used to configure the project for a specific architecture and build type.
*   `scripts/actions/build_alkos.bash` - Builds the AlkOS kernel and creates an ISO image.
*   `scripts/actions/make_iso.bash` - Creates an ISO image from the built kernel and modules.
*   `scripts/actions/run_alkos.bash` - Runs AlkOS in QEMU.
*   `scripts/actions/attach_to_qemu_gdb.bash` - Attaches GDB to a running QEMU instance for debugging the kernel.
*   `scripts/actions/apply_clang_format.bash` - Applies clang-format to the source code to maintain consistent code style.
*   `scripts/tests/runner.py` - Entry point for the test framework.
*   `scripts/env/build_cross_compile.bash` - Script for building the cross-compiler.
*   `scripts/git-hooks/setup-hooks.bash` - Script for setting up git hooks.
*   `scripts/git-hooks/pre-commit` - Pre-commit hook for clang-format.

## Getting Started

### 📋 Prerequisites

*   A system running **Arch Linux** or **Ubuntu** for installing dependencies. Otherwise you will need to install the dependencies in `scripts/env/arch_packages.txt` manually.
*   A **Unix-like environment** for running the scripts (bash shell)
*   **CMake** >= 3.30: Used as the build system.
*   **QEMU:** Used for emulating the x86_64 architecture and running AlkOS.
*   **NASM:** Used as the assembler for assembly code.
*   **grub-mkrescue:** Used for creating bootable ISO images.
*   **wget:** Used by the installation scripts for downloading necessary source files.
*   A cross-compilation toolchain for x86_64-elf (installed by `alkos_cli.bash`).
*   **make**: Used for building the project.
*   **ld** (the GNU linker): Used for linking the project.

### 🛠️ Installation: Dependencies and Toolchain

1.  **Clone the repository:**

    ```bash
    git clone [https://github.com/Jlisowskyy/AlkOS](https://github.com/Jlisowskyy/AlkOS)
    ```

2.  **Navigate to the scripts directory:**

    ```bash
    cd AlkOS/scripts
    ```

3.  **Configure the build:**
    You need to configure the build using the `configure.bash` script. This script sets up the build environment for a specific architecture and build type.

    For example, to configure a debug build for QEMU with tests enabled, run:

    ```
    ./configure.bash x86_64 debug_qemu_tests -v
    ```

    This will create a `conf.bash` file in the `scripts` directory. This file is sourced by other scripts to get configuration information. The configuration script will 
    also create a build directory at `../build`.

    To see all available configuration options, run:

    ```
    ./configure.bash -h
    ```

4.  **Install dependencies and the cross-compilation toolchain:**

    ```bash
    ./alkos_cli.bash --install deps -v
    ./alkos_cli.bash --install toolchain -v
    ```
    or:
    ```bash
    ./alkos_cli.bash -i all -v
    ```

    This step will install all necessary dependencies and set up the toolchain for the architecture set with `configure.bash`.

    Note: Installing the dependencies assumes you are running on Arch Linux. If not, you will need to manually install the packages listed in `scripts/env/arch_packages.txt`.

    Note 2: There exists a dependencies installer for Ubuntu: `./scripts/env/install_deps_ubuntu.bash`

## 💻 Development Workflow

### 🏗️ Building AlkOS

1.  **Navigate to the scripts directory:**

    ```bash
    cd AlkOS/scripts
    ```

2.  **Build the project:**
    The build system uses CMake. When you run `alkos_cli.bash` with `-r`, it first invokes the `build_alkos.bash` script. This script creates a build directory, 
    generates Makefiles using CMake with the specified cross-compilation toolchain, and then builds the project using `make`.

    ```
    ./alkos_cli.bash --verbose --run
    ```

    or:

    ```
    ./alkos_cli.bash -v -r
    ```

    Note: This assumes you have the necessary dependencies, generated a config using `configure.bash` and have a cross-compiler toolchain for the chosen architecture.


### ▶️ Running AlkOS in QEMU

The `-r` flag of `alkos_cli.bash` also handles running AlkOS in QEMU after building it. It invokes `run_alkos.bash` script. This script will start QEMU with the correct parameters, including booting from the generated ISO image.

   ```bash
   ./alkos_cli.bash -v -r
   ```

### Setting up pre-commit hooks

The `-g` flag of `alkos_cli.bash` sets up default git hook dir of this repository to `scripts/git-hooks`
   ```bash
   ./alkos_cli.bash -g
   ```

### 🧪 Running Tests

AlkOS includes a custom testing framework inspired by Google Test. Tests are defined within the kernel code and are executed in the QEMU environment.

1.  **Navigate to the scripts directory:**

    ```
    cd AlkOS/scripts
    ```

2.  **Run the test framework:**

    ```
    ./actions/run_tests.bash
    ```

    The build process must have been run at least once before running the tests, also the test configuration must have been set.
    Changing the configuration to one which supports tests requires a rebuild before running this script.

### 🐛 Debugging with GDB

1.  **Build the project in debug mode (default when using `debug_qemu` or `debug_qemu_tests`).**
2.  **Run AlkOS in QEMU with GDB debugging enabled:**

    ```
    ./actions/run_alkos.bash -v -g
    ```

    This will start QEMU with the `-s` and `-S` flags, which will make QEMU wait for a GDB connection on port 1234.
3.  **In a separate terminal, attach GDB to the running QEMU instance:**

    ```
    ./install/attach_to_qemu_gdb.bash <path_to_kernel_binary> -g <path_to_gdb>
    ```

    Replace `<path_to_kernel_binary>` with the path to your compiled kernel binary (e.g. `../build/alkos/sysroot/boot/alkos.kernel`).
    Replace `<path_to_gdb>` with the path to your GDB executable appropiate for the architecture (in tools).

### 🎨 Applying clang-format for Code Style

1.  **Navigate to the scripts directory:**

    ```bash
    cd AlkOS/scripts
    ```

2.  **Run the clang-format script:**

    ```bash
    ./actions/apply_clang_format.bash
    ```

    This will format all C++ and C source files according to the project's style defined in `.clang-format` file.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
