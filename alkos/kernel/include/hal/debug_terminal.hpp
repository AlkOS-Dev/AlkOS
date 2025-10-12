#ifndef ALKOS_KERNEL_INCLUD_HAL_DEBUG_TERMINAL_HPP_
#define ALKOS_KERNEL_INCLUD_HAL_DEBUG_TERMINAL_HPP_

#include <hal/impl/debug_terminal.hpp>

namespace hal
{

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

}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUD_HAL_DEBUG_TERMINAL_HPP_
