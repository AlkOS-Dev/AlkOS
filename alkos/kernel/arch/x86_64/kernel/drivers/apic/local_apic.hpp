#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_LOCAL_APIC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_LOCAL_APIC_HPP_

#include <cpuid.h>
#include <extensions/defines.hpp>
#include <extensions/types.hpp>

static constexpr u32 kEdxAcpiFlag = 1 << 9;

NODISCARD FAST_CALL bool IsApicSupported()
{
    unsigned int edx;
    unsigned int unused;

    __get_cpuid(1, &unused, &unused, &unused, &edx);
    return edx & kEdxAcpiFlag;
}

void EnableLocalAPIC();

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_LOCAL_APIC_HPP_
