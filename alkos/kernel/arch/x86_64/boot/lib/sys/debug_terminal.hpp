#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_DEBUG_sys/terminal.hpp_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_DEBUG_sys/terminal.hpp_

/* internal includes */
#include <assert.h>
#include <autogen/feature_flags.h>
#include <drivers/serial/qemu.hpp>
#include <extensions/defines.hpp>

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

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_DEBUG_sys/terminal.hpp_
