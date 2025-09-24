#ifndef ALKOS_KERNEL_ABI_DEBUG_TERMINAL_HPP_
#define ALKOS_KERNEL_ABI_DEBUG_TERMINAL_HPP_

#include <autogen/feature_flags.h>
#include <extensions/defines.hpp>
#include "extensions/types.hpp"

/* Should be implemented inside architecture-specific code */
namespace arch
{
/**
 * @brief Reads a line from the architecture-specific debug terminal.
 * @param buffer The buffer to store the read data.
 * @param buffer_size The size of the buffer.
 * @return The number of characters read (including null terminator).
 *
 * @note This function should block until something is read from the terminal.
 */
WRAP_CALL size_t DebugTerminalReadLine(char *buffer, size_t buffer_size);

/**
 * @brief Writes a string to the architecture-specific debug terminal.
 * @param buffer The string to be written.
 * @note This function should block until something is written to the terminal.
 */
WRAP_CALL void DebugTerminalWrite(const char *buffer);
}  // namespace arch

WRAP_CALL void DebugTerminalWrite(const char *str)
{
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        arch::DebugTerminalWrite(str);
    }
}

WRAP_CALL size_t DebugTerminalReadLine(char *const buffer, const size_t buffer_size)
{
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        return arch::DebugTerminalReadLine(buffer, buffer_size);
    }

    return 0;
}

#include <hal/debug_terminal.hpp>

#endif  // ALKOS_KERNEL_ABI_DEBUG_TERMINAL_HPP_
