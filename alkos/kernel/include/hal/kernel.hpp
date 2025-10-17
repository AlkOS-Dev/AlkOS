#ifndef ALKOS_KERNEL_INCLUDE_HAL_KERNEL_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_KERNEL_HPP_

#include <hal/impl/kernel.hpp>

namespace hal
{
using arch::RawKernelArguments;

WRAP_CALL void ArchInit(const RawKernelArguments& args) { arch::ArchInit(args); }
}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_KERNEL_HPP_
