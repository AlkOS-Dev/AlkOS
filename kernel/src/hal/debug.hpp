#ifndef ALKOS_KERNEL_INCLUDE_HAL_DEBUG_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_DEBUG_HPP_

#include <hal/impl/debug.hpp>

namespace hal
{
/**
 * @brief Display some debug info for stack
 */
WRAP_CALL void DebugStack() { arch::DebugStack(); }
}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_DEBUG_HPP_
