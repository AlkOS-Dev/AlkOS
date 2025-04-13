#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_DEBUG_TERMINAL_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_DEBUG_TERMINAL_HPP_

/* internal includes */
#include <assert.h>
#include <defines.hpp>
#include <drivers/serial_port_qemu/serial_qemu.hpp>

namespace arch
{
WRAP_CALL void DebugTerminalWrite(const char *const buffer)
{
    /* verify if the usage is permitted */
    R_ASSERT_TRUE(kUseDebugOutput);

    QemuTerminalWriteString(buffer);
}

WRAP_CALL size_t DebugTerminalReadLine(char *const buffer, const size_t buffer_size)
{
    /* verify if the usage is permitted */
    R_ASSERT_TRUE(kUseDebugOutput);

    return QemuTerminalReadLine(buffer, buffer_size);
}
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_DEBUG_TERMINAL_HPP_
