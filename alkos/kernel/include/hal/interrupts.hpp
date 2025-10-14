#ifndef ALKOS_KERNEL_INCLUDE_HAL_INTERRUPTS_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_INTERRUPTS_HPP_

#include <hal/impl/interrupts.hpp>

namespace hal
{
using arch::Interrupts;

/* Should contain data for interrupts mechanism to modify page fault handler */
static constexpr size_t kPageFaultIsrId = arch::kPageFaultIsrId;

}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_INTERRUPTS_HPP_
