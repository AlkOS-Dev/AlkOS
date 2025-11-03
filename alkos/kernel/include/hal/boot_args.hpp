#ifndef ALKOS_KERNEL_INCLUDE_HAL_BOOT_ARGS_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_BOOT_ARGS_HPP_

#include <hal/impl/kernel.hpp>

namespace hal
{
using arch::RawBootArguments;

WRAP_CALL void ArchInit(const RawBootArguments &args) { arch::ArchInit(args); }
}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_BOOT_ARGS_HPP_
