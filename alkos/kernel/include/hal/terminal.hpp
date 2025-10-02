#ifndef ALKOS_KERNEL_INCLUDE_HAL_TERMINAL_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_TERMINAL_HPP_

#include <hal/impl/terminal.hpp>

namespace hal
{
WRAP_CALL void TerminalInit() { arch::TerminalInit(); }
WRAP_CALL void TerminalPutChar(const char c) { arch::TerminalPutChar(c); }
WRAP_CALL void TerminalWriteString(const char *data) { arch::TerminalWriteString(data); }
WRAP_CALL void TerminalWriteError(const char *data) { arch::TerminalWriteError(data); }
WRAP_CALL char TerminalGetChar() { return arch::TerminalGetChar(); }
WRAP_CALL size_t TerminalReadLine(char *buffer, const size_t size)
{
    return arch::TerminalReadLine(buffer, size);
}

}  


#endif // ALKOS_KERNEL_INCLUDE_HAL_TERMINAL_HPP_
