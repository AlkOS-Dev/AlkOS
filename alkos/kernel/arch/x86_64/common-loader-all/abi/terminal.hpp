#ifndef ARCH_X86_64_COMMON_ABI_TERMINAL_HPP_
#define ARCH_X86_64_COMMON_ABI_TERMINAL_HPP_

/* internal includes */
#include <defines.hpp>
#include <terminal.hpp>

#include <drivers/serial_port_qemu/serial_qemu.hpp>
#include <drivers/vga/vga.hpp>
#include <todo.hpp>

extern "C" {
WRAP_CALL void TerminalPutChar(const char c)
{
    /* Put char to VGA terminal -> when multiboot allows: TODO */
    // VgaTerminalPutChar(c);

    if constexpr (kUseDebugOutput) {
        QemuTerminalPutChar(c);
    }
}

WRAP_CALL void TerminalWriteString(const char *data)
{
    /* Write string to VGA terminal -> when multiboot allows: TODO */
    // VgaTerminalWriteString(data);

    if constexpr (kUseDebugOutput) {
        QemuTerminalWriteString(data);
    }
}

WRAP_CALL void TerminalWriteError(const char *data)
{
    /* Write error string to VGA terminal -> when multiboot allows: TODO */
    // VgaTerminalWriteError(data);

    if constexpr (kUseDebugOutput) {
        QemuTerminalWriteString(data);
    }
}

WRAP_CALL char TerminalGetChar()
{
    if constexpr (kUseDebugOutput) {
        return QemuTerminalGetChar();
    }

    TODO_BY_THE_END_OF_MILESTONE0
    return 'x';
}

WRAP_CALL size_t TerminalReadLine(char *buffer, const size_t size)
{
    if constexpr (kUseDebugOutput) {
        return QemuTerminalReadLine(buffer, size);
    }

    TODO_BY_THE_END_OF_MILESTONE0
    return 0;
}
}

#endif  // ARCH_X86_64_COMMON_ABI_TERMINAL_HPP_
