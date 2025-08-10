#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_TSC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_TSC_HPP_

#include <cpu/control_registers.hpp>

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
 */
namespace tsc
{

FORCE_INLINE_F void SetUserSpaceAccess(const bool enabled) {}

void Initialize() {}

}  // namespace tsc

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_TSC_HPP_
