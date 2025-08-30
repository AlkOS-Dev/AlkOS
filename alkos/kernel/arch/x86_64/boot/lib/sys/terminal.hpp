#ifndef ALKOS_BOOT_LIB_SYS_TERMINAL_HPP_
#define ALKOS_BOOT_LIB_SYS_TERMINAL_HPP_

/* internal includes */
#include <autogen/feature_flags.h>
#include <extensions/defines.hpp>

#include <todo.hpp>
#include "hw/serial/qemu.hpp"
#include "hw/vga.hpp"

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

/**
 * @brief Reads a single character from the terminal input.
 * @return The character read from input.
 * @note Must block until a character is available.
 */
WRAP_CALL char TerminalGetChar();

/**
 * @brief Reads a line of input from the terminal into a buffer.
 * @param buffer The buffer to store the input.
 * @param size The maximum number of characters to read (including null terminator).
 * @return The number of characters read, including the null terminator.
 * @note Must block until a character is available.
 */
WRAP_CALL size_t TerminalReadLine(char *buffer, size_t size);
}

namespace arch
{
WRAP_CALL void TerminalInit() { ::TerminalInit(); }
WRAP_CALL void TerminalPutChar(const char c) { ::TerminalPutChar(c); }
WRAP_CALL void TerminalWriteString(const char *data) { ::TerminalWriteString(data); }
WRAP_CALL void TerminalWriteError(const char *data) { ::TerminalWriteError(data); }
WRAP_CALL char TerminalGetChar() { return ::TerminalGetChar(); }
WRAP_CALL size_t TerminalReadLine(char *buffer, const size_t size)
{
    return ::TerminalReadLine(buffer, size);
}
}  // namespace arch

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

WRAP_CALL char TerminalGetChar()
{
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        return QemuTerminalGetChar();
    }

    TODO_BY_THE_END_OF_MILESTONE0
    return 'x';
}

WRAP_CALL size_t TerminalReadLine(char *buffer, const size_t size)
{
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        return QemuTerminalReadLine(buffer, size);
    }

    TODO_BY_THE_END_OF_MILESTONE0
    return 0;
}
}

#endif  // ALKOS_BOOT_LIB_SYS_TERMINAL_HPP_
