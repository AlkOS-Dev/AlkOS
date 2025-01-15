#ifndef KERNEL_ABI_DEBUG_TERMINAL_HPP_
#define KERNEL_ABI_DEBUG_TERMINAL_HPP_

#include <defines.hpp>
#include <types.hpp>

/* Should be implemented inside architecture-specific code */
extern "C" {
/**
 * @brief Reads a line from the architecture-specific debug terminal.
 * @param buffer The buffer to store the read data.
 * @param buffer_size The size of the buffer.
 * @return The number of characters read (including null terminator).
 *
 * @note This function should block until something is read from the terminal.
 */
WRAP_CALL size_t DebugTerminalReadLineArch_(char *buffer, size_t buffer_size);

/**
 * @brief Writes a string to the architecture-specific debug terminal.
 * @param buffer The string to be written.
 * @note This function should block until something is written to the terminal.
 */
WRAP_CALL void DebugTerminalWriteArch_(const char *buffer);
}

WRAP_CALL void DebugTerminalWrite(const char *str)
{
    if constexpr (kUseDebugOutput) {
        DebugTerminalWriteArch_(str);
    }
}

WRAP_CALL size_t DebugTerminalReadLine(char *const buffer, const size_t buffer_size)
{
    if constexpr (kUseDebugOutput) {
        return DebugTerminalReadLineArch_(buffer, buffer_size);
    }

    return 0;
}

#include <abi/debug_terminal.hpp>

#endif  // KERNEL_ABI_DEBUG_TERMINAL_HPP_
