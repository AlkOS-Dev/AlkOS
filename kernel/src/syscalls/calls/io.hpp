#ifndef KERNEL_SRC_SYSCALLS_CALLS_IO_HPP_
#define KERNEL_SRC_SYSCALLS_CALLS_IO_HPP_

#include <defines.h>
#include <types.h>

#include "hal/debug_terminal.hpp"
#include "trace_framework.hpp"

namespace Syscall
{

// ------------------------------
// Debug IO Syscalls
// ------------------------------

/**
 * @brief Write string to debug output
 * @param buffer String to write (null-terminated)
 */
FORCE_INLINE_F void SysDebugWrite(const char *buffer) { DEBUG_INFO_GENERAL(buffer); }

/**
 * @brief Read line from debug input
 * @param buffer Buffer to store input
 * @param buffer_size Size of buffer
 * @return Number of characters read
 */
FORCE_INLINE_F u64 SysDebugReadLine(char *buffer, size_t buffer_size)
{
    return hal::DebugTerminalReadLine(buffer, buffer_size);
}

}  // namespace Syscall

#endif  // KERNEL_SRC_SYSCALLS_CALLS_IO_HPP_
