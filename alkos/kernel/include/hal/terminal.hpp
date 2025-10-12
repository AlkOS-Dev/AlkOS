#ifndef ALKOS_KERNEL_INCLUDE_HAL_TERMINAL_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_TERMINAL_HPP_

#include <hal/impl/terminal.hpp>

namespace hal
{
using arch::TerminalGetChar;
using arch::TerminalInit;
using arch::TerminalPutChar;
using arch::TerminalReadLine;
using arch::TerminalWriteError;
using arch::TerminalWriteString;
}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_TERMINAL_HPP_
