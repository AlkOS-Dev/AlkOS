#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_TERMINAL_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_TERMINAL_HPP_

#include <hal/api/terminal.hpp>

#include <autogen/feature_flags.h>
#include <extensions/defines.hpp>

#include <todo.hpp>

#include "drivers/serial/qemu.hpp"
#include "drivers/vga/vga.hpp"

namespace arch {
BEGIN_DECL_C
WRAP_CALL void TerminalInit() {
    if constexpr (FeatureEnabled<FeatureFlag::kDebugOutput>) {
        QemuTerminalInit();
    }
}

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
END_DECL_C
} 

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_ABI_TERMINAL_HPP_
