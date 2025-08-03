#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_MSRS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_MSRS_HPP_

#include <extensions/types.hpp>

FAST_CALL u64 CpuGetMSR(const u32 msr)
{
    u32 lo;
    u32 hi;

    asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));

    return (static_cast<u64>(hi)) << 32 | lo;
}

FAST_CALL void CpuSetMSR(const u32 msr, const u64 value)
{
    const u32 lo = static_cast<u32>(value);
    const u32 hi = static_cast<u32>(value >> 32);

    asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_MSRS_HPP_
