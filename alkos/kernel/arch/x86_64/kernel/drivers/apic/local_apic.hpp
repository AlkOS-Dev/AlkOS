#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_LOCAL_APIC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_LOCAL_APIC_HPP_

#include <assert.h>
#include <cpuid.h>
#include <extensions/bit.hpp>
#include <extensions/defines.hpp>
#include <extensions/types.hpp>
#include "msrs.hpp"

static constexpr u32 kEdxAcpiFlag = 1 << 9;

static constexpr u32 kIA32ApicBaseMsr = 0x1B;

static constexpr u64 kIA32ApicBaseMsrEnable = 0x800;

NODISCARD FAST_CALL bool IsApicSupported()
{
    unsigned int edx;
    unsigned int unused;

    __get_cpuid(1, &unused, &unused, &unused, &edx);
    return edx & kEdxAcpiFlag;
}

NODISCARD FAST_CALL u64 GetLocalApicPhysicalAddress()
{
    /* Return 4k page aligned address */
    return CpuGetMSR(kIA32ApicBaseMsr) & ~kBitMaskRight<u64, 12>;
}

FAST_CALL void SetLocalApicPhysicalAddress(const u64 new_address)
{
    ASSERT_TRUE(IsAligned(new_address, 12), "Local APIC address is not aligned to 4k page!");
    CpuSetMSR(kIA32ApicBaseMsr, new_address | kIA32ApicBaseMsrEnable);
}

void EnableLocalAPIC();

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_LOCAL_APIC_HPP_
