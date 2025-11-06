#ifndef KERNEL_ARCH_X86_64_SRC_CPU_MSRS_HPP_
#define KERNEL_ARCH_X86_64_SRC_CPU_MSRS_HPP_

#include <types.hpp>

namespace cpu
{
NODISCARD FAST_CALL u64 GetMSR(const u32 msr)
{
    u32 lo;
    u32 hi;

    asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));

    return (static_cast<u64>(hi)) << 32 | lo;
}

FAST_CALL void SetMSR(const u32 msr, const u64 value)
{
    const u32 lo = static_cast<u32>(value);
    const u32 hi = static_cast<u32>(value >> 32);

    asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}
}  // namespace cpu

#endif  // KERNEL_ARCH_X86_64_SRC_CPU_MSRS_HPP_
