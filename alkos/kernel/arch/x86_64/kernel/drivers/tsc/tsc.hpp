#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_TSC_TSC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_TSC_TSC_HPP_

#include <cpuid.h>
#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <extensions/types.hpp>
#include <trace.hpp>
#include "cpu/control_registers.hpp"
#include "cpu/msrs.hpp"
#include "todo.hpp"

TODO_WHEN_MULTITHREADING
// TODO: Ensure TSC is synchronised across cores after launching using MSRs

TODO_WHEN_TIMER_INFRA_DONE
// TODO: Calculate TSC frequency for infra

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
 * Volume 3A: System Programming Guide, Part 1. Chapter 19.17
 *
 * @note Possibly in future we should allow usage of RDTSC in user space
 */
namespace tsc
{
// ------------------------------
// defines
// ------------------------------

static constexpr u64 kIA32TscMsrAddress     = 0x10;
static constexpr u64 kCpuIdTscInvariantLeaf = 0x80000007;

// ------------------------------
// Utility functions
// ------------------------------

FAST_CALL void SetUserSpaceAccess(const bool enabled)
{
    auto cr3             = cpu::GetCR<cpu::Cr4>();
    cr3.TimeStampDisable = !enabled;
    cpu::SetCR(cr3);
}

NODISCARD FAST_CALL u64 Read()
{
    u32 lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<u64>(hi) << 32) | lo;
}

NODISCARD FAST_CALL u64 ReadViaMsr() { return cpu::GetMSR(kIA32TscMsrAddress); }

FAST_CALL void Write(const u64 value) { cpu::SetMSR(kIA32TscMsrAddress, value); }

NODISCARD FAST_CALL bool IsAvailable()
{
    u32 unused, edx;

    if (__get_cpuid(1, &unused, &unused, &unused, &edx)) {
        // Bit 4 of EDX indicates TSC support
        return IsBitEnabled<4>(edx);
    }

    return false;
}

NODISCARD FAST_CALL bool IsStable()
{
    u32 unused, edx;

    if (__get_cpuid(kCpuIdTscInvariantLeaf, &unused, &unused, &unused, &edx)) {
        // Bit 8 of EDX indicates TSC invariant
        return IsBitEnabled<8>(edx);
    }

    return false;
}

// ------------------------------
// Kernel flow functions
// ------------------------------

void Initialize();
}  // namespace tsc

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_TSC_TSC_HPP_
