#ifndef ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_SYS_TERMINAL_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_SYS_TERMINAL_HPP_

#include <autogen/feature_flags.h>
#include <extensions/defines.hpp>

#include "hw/serial/qemu.hpp"
#include "hw/vga.hpp"
#include "todo.hpp"

extern "C" {
/**
 * @brief Initializes the terminal for input and output operations.
 * @note Must set up any required buffers or hardware state.
 */
void TerminalInit();

/**
 * @brief Outputs a single character to the terminal.
 * @param c The character to output.
 * @note Must ensure the character is displayed immediately.
 */
WRAP_CALL void TerminalPutChar(char c);

/**
 * @brief Writes a null-terminated string to the terminal.
 * @param data The string to write.
 * @note Must handle newlines and special characters appropriately. (TODO)
 */
WRAP_CALL void TerminalWriteString(const char *data);

/**
 * @brief Writes an error message to the terminal.
 * @param data The error string to write.
 * @note Should visually distinguish error messages (e.g., with a specific color or prefix).
 */
WRAP_CALL void TerminalWriteError(const char *data);
}

extern "C" {

WRAP_CALL void TerminalPutChar(const char c)
{
    /* Put char to VGA terminal -> when multiboot allows: TODO */
    // VgaTerminalPutChar(c);

    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        QemuTerminalPutChar(c);
    }
}

WRAP_CALL void TerminalWriteString(const char *data)
{
    /* Write string to VGA terminal -> when multiboot allows: TODO */
    // VgaTerminalWriteString(data);

    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        QemuTerminalWriteString(data);
    }
}

WRAP_CALL void TerminalWriteError(const char *data)
{
    /* Write error string to VGA terminal -> when multiboot allows: TODO */
    // VgaTerminalWriteError(data);

    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        QemuTerminalWriteString(data);
    }
}
}

#endif  // ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_SYS_TERMINAL_HPP_
