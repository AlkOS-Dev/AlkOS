#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_CPU_CONTROL_REGISTERS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_CPU_CONTROL_REGISTERS_HPP_

#include <extensions/types.hpp>

namespace cpu
{
enum class ControlRegister : u8 { CR0, CR2, CR3, CR4 };

template <ControlRegister reg>
u64 GetCR()
{
    u64 value;
    if constexpr (reg == ControlRegister::CR0) {
        asm volatile("mov %%cr0, %0" : "=r"(value));
    } else if constexpr (reg == ControlRegister::CR2) {
        asm volatile("mov %%cr2, %0" : "=r"(value));
    } else if constexpr (reg == ControlRegister::CR3) {
        asm volatile("mov %%cr3, %0" : "=r"(value));
    } else if constexpr (reg == ControlRegister::CR4) {
        asm volatile("mov %%cr4, %0" : "=r"(value));
    }
    return value;
}

template <ControlRegister reg>
void SetCR(u64 value)
{
    if constexpr (reg == ControlRegister::CR0) {
        asm volatile("mov %0, %%cr0" : : "r"(value));
    } else if constexpr (reg == ControlRegister::CR2) {
        asm volatile("mov %0, %%cr2" : : "r"(value));
    } else if constexpr (reg == ControlRegister::CR3) {
        asm volatile("mov %0, %%cr3" : : "r"(value));
    } else if constexpr (reg == ControlRegister::CR4) {
        asm volatile("mov %0, %%cr4" : : "r"(value));
    }
}

}  // namespace cpu

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_CPU_CONTROL_REGISTERS_HPP_
