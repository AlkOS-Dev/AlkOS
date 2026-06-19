// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_ARCH_X86_64_BOOT_LIB_HW_SERIAL_QEMU_HPP_
#define KERNEL_ARCH_X86_64_BOOT_LIB_HW_SERIAL_QEMU_HPP_

#include <types.h>

/**
 * @file serial_qemu.hpp
 * @brief Interface for QEMU serial port communication
 *
 * This header provides the interface for initializing and using a serial port
 * for communication with QEMU's terminal. It supports basic I/O operations
 * including character and string transmission, as well as line-based reading.
 */

extern "C" {
void QemuTerminalInit();
void QemuTerminalPutChar(char c);
void QemuTerminalWriteString(const char *s);
char QemuTerminalGetChar();
size_t QemuTerminalReadLine(char *buffer, size_t size);
}

#endif  // KERNEL_ARCH_X86_64_BOOT_LIB_HW_SERIAL_QEMU_HPP_
