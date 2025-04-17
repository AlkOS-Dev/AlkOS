#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_

#include <interrupts.hpp>

namespace hardware
{
class Interrupts final : public arch::Interrupts
{
};
}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_INTERUPTS_HPP_
