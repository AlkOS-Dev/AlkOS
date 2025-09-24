#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_DEBUG_TERMINAL_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_DEBUG_TERMINAL_HPP_

/* internal includes */
#include <assert.h>
#include <autogen/feature_flags.h>
#include <drivers/serial_port_qemu/serial_qemu.hpp>
#include <extensions/defines.hpp>

namespace arch
{
WRAP_CALL void DebugTerminalWrite(const char *const buffer)
{
    /* verify if the usage is permitted */
    ASSERT_TRUE(FeatureEnabled<FeatureFlag::kDebugOutput>);

    QemuTerminalWriteString(buffer);
}

WRAP_CALL size_t DebugTerminalReadLine(char *const buffer, const size_t buffer_size)
{
    /* verify if the usage is permitted */
    ASSERT_TRUE(FeatureEnabled<FeatureFlag::kDebugOutput>);

    return QemuTerminalReadLine(buffer, buffer_size);
}
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_DEBUG_TERMINAL_HPP_
