#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_CPU_CONTROL_REGISTERS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_CPU_CONTROL_REGISTERS_HPP_

#include <extensions/types.hpp>

namespace cpu
{
enum class ControlRegister : u8 { CR0, CR2, CR3, CR4 };

}  // namespace cpu

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_CPU_CONTROL_REGISTERS_HPP_
