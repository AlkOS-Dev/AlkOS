#ifndef KERNEL_SRC_HAL_DEBUG_HPP_
#define KERNEL_SRC_HAL_DEBUG_HPP_

#include <hal/impl/debug.hpp>

namespace hal
{
/**
 * @brief Display some debug info for stack
 */
WRAP_CALL void DebugStack() { arch::DebugStack(); }
WRAP_CALL void Noop() { arch::Noop(); }
}  // namespace hal

#endif  // KERNEL_SRC_HAL_DEBUG_HPP_
