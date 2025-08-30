#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_DRIVERS_SERIAL_PORT_QEMU_SERIAL_QEMU_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_DRIVERS_SERIAL_PORT_QEMU_SERIAL_QEMU_HPP_

#include <extensions/types.hpp>

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

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_DRIVERS_SERIAL_PORT_QEMU_SERIAL_QEMU_HPP_
