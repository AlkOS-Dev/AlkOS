#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_TSC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_TSC_HPP_

#include <cpuid.h>
#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <extensions/types.hpp>
#include <todo.hpp>
#include <trace.hpp>
#include "cpu/control_registers.hpp"

/**
 * @file tsc.hpp
 * @brief Time Stamp Counter (TSC) driver for x86_64.
 *
 * The Time Stamp Counter (TSC) is a 64-bit register present in all x86
 * processors since the Pentium. It counts CPU cycles since the last reset,
 * providing a high-resolution, low-overhead mechanism for timing. The RDTSC
 * instruction reads the current value of the TSC.
 *
 * Modern Intel processors feature an invariant TSC, which increments at a
 * constant rate regardless of changes in the CPU's core frequency. This makes
 * it a reliable source for timekeeping.
 *
 * @see: Intel 64 and IA-32 Architectures Software Developer's Manual,
 * Volume 3A: System Programming Guide, Part 1.
 *
 * @note Possibly in future we should allow usage of RDTSC in user space
 */
namespace tsc
{

// ------------------------------
// Utility functions
// ------------------------------

FORCE_INLINE_F void SetUserSpaceAccess(const bool enabled)
{
    auto cr3             = cpu::GetCR<cpu::Cr4>();
    cr3.TimeStampDisable = !enabled;
    cpu::SetCR(cr3);
}

NODISCARD FORCE_INLINE_F uint64_t Read()
{
    u32 lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<u64>(hi) << 32) | lo;
}

NODISCARD FORCE_INLINE_F bool IsAvailable()
{
    u32 unused, edx;

    if (__get_cpuid(1, &unused, &unused, &unused, &edx)) {
        // Bit 4 of EDX indicates TSC support
        return IsBitEnabled<4>(edx);
    }

    return false;
}

// ------------------------------
// Kernel flow functions
// ------------------------------

void Initialize()
{
    if (!IsAvailable()) {
        KernelTraceInfo("TSC is not available. Fallback to old technology...");
        return;
    }

    // NOTE: disabled RDTSC in user space
    SetUserSpaceAccess(false);

    TRACE_DEBUG("Detected TSC, current counter: %zu", Read());

    TODO_WHEN_TIMER_INFRA_DONE
    // TODO: add to the infra
}

}  // namespace tsc

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_TSC_HPP_
