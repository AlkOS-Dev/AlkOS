#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_LOCAL_APIC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_LOCAL_APIC_HPP_

#include <assert.h>
#include <cpuid.h>
#include <extensions/bit.hpp>
#include <extensions/defines.hpp>
#include <extensions/types.hpp>
#include <todo.hpp>
#include "memory_io.hpp"
#include "msrs.hpp"

/**
 *  Local IO APIC driver
 *
 *  Reference: Intel System Programming Guide, Vol 3A Part 1, Chapter 10
 */
namespace LocalApic
{
// ------------------------------
// defines
// ------------------------------

/**
 * Architecture flags
 */

static constexpr u32 kEdxAcpiFlag           = 1 << 9;
static constexpr u32 kIA32ApicBaseMsr       = 0x1B;
static constexpr u64 kIA32ApicBaseMsrEnable = 0x800;

/**
 * Local APIC register details:
 */

/* RW - Read Write, RO - Read Only, WO - Write Only */
/* All registers are 32 bit wide, but aligned to 16 byte boundary */

/* 0x000-0x010 - RESERVED */
static constexpr u32 kIdRegRW      = 0x020;
static constexpr u32 kVersionRegRO = 0x030;
/* 0x040 - 0x070 - RESERVED */
static constexpr u32 kTaskPriorityRegRW        = 0x080; /* TPR */
static constexpr u32 kArbitrationPriorityRegRO = 0x090; /* APR */
static constexpr u32 kProcessorPriorityRegRO   = 0x0A0; /* PPR */
static constexpr u32 kEndOfInterruptRegWO      = 0x0B0; /* EOI */
static constexpr u32 kRemoteReadRegRO          = 0x0C0; /* RRD */
static constexpr u32 kLogicalDestinationRegRW  = 0x0D0;
static constexpr u32 kDestinationFormatRegRW   = 0x0E0;
static constexpr u32 kSpuriousInterruptRegRW   = 0x0F0;
/* 0x100 - 0x170 - In-Service Register - ISR */
/* 0x180 - 0x1F0 - Trigger Mode Register - TMR */
/* 0x200 - 0x270 - Interrupt Request Register - IRR */
static constexpr u32 kErrorStatusRegRO = 0x280;
/* 0x290 - 0x2E0 - RESERVED */
static constexpr u32 kLvtCmciRegRW               = 0x2F0; /* LVT CMCI */
static constexpr u32 kInterruptCommandLowRegRW   = 0x300; /* ICR Low */
static constexpr u32 kInterruptCommandHighRegRW  = 0x310; /* ICR High */
static constexpr u32 kLvtTimerRegRW              = 0x320;
static constexpr u32 kLvtThermalSensorRegRW      = 0x330;
static constexpr u32 kLvtPerformanceCounterRegRW = 0x340;
static constexpr u32 kLvtLint0RegRW              = 0x350;
static constexpr u32 kLvtLint1RegRW              = 0x360;
static constexpr u32 kLvtErrorRegRW              = 0x370;
static constexpr u32 kInitialCountRegRW          = 0x380; /* Timer */
static constexpr u32 kCurrentCountRegRO          = 0x390; /* Timer */
/* 0x3A0 - 0x3D0 - RESERVED */
static constexpr u32 kDivideConfigRegRW = 0x3E0; /* Timer */
/* 0x3F0 - RESERVED */

/**
 * EOI
 */

static constexpr u32 kEOISignal = 0;

/**
 * Local Vector Table Registers
 */

struct LocalVectorTableRegister {
    u32 vector : 8;
    u32 reserved1 : 4;
    u32 pending : 1; /* set if pending */
    u32 reserved2 : 3;
    u32 mask : 1; /* Set to disable */
    u32 reserved3 : 15;
};
static_assert(sizeof(LocalVectorTableRegister) == 4);

struct LocalVectorTableTimerRegister {
    u32 vector : 8;
    u32 type : 3;
    u32 reserved1 : 1;
    u32 pending : 1;  /* set if pending */
    u32 polarity : 1; /* set to low triggered */
    u32 remote_irr : 1;
    u32 trigger_mode : 1; /* set for level triggered */
    u32 mask : 1;         /* Set to disable */
    u32 reserved2 : 15;

    static constexpr u32 kTypeNMI = 0x100b;
};
static_assert(sizeof(LocalVectorTableTimerRegister) == 4);

/**
 * Spurious Interrupt Vector Register
 */

struct SpuriousInterruptRegister {
    u32 vector : 8;
    u32 enabled : 1;
    u32 reserved1 : 3;
    u32 no_eoi_broadcast : 1;
    u32 reserved2 : 19;
};
static_assert(sizeof(SpuriousInterruptRegister) == 4);

/**
 * Interrupt Command Register
 */

struct InterruptCommandRegister {
    u32 vector : 8;
    u32 delivery_mode : 3;
    u32 destination_mode : 1;
    u32 delivery_status : 1;
    u32 reserved1 : 1;
    u32 init_type : 2;
    u32 destination_type : 2;
    u32 reserved2 : 12;

    enum class DeliveryMode : u8 {
        kFixed         = 0b000,
        kLowerPriority = 0b001,
        kSMI           = 0b010,
        kNMI           = 0b100,
        kINIT          = 0b101,
        kSIPI          = 0b110,
    };

    enum class DestinationMode : u8 {
        kPhysical = 0,
        kLogical  = 1,
    };

    enum class InitType : u8 {
        kNormal   = 0b01,
        kDeAssert = 0b10,
    };

    enum class DestinationType : u8 {
        kNormal                  = 0,
        kNotifyYourself          = 1,
        kBroadcast               = 2,
        kBroadcastExceptYourself = 3
    };
};
static_assert(sizeof(InterruptCommandRegister) == 4);

// ------------------------------
// Utility functions
// ------------------------------

NODISCARD FAST_CALL bool IsSupported()
{
    unsigned int edx;
    unsigned int unused;

    __get_cpuid(1, &unused, &unused, &unused, &edx);
    return edx & kEdxAcpiFlag;
}

NODISCARD FAST_CALL u64 GetPhysicalAddress()
{
    /* Return 4k page aligned address */
    return CpuGetMSR(kIA32ApicBaseMsr) & ~kBitMaskRight<u64, 12>;
}

FAST_CALL void SetPhysicalAddress(const u64 new_address)
{
    ASSERT_TRUE(IsAligned(new_address, 12), "Local APIC address is not aligned to 4k page!");
    CpuSetMSR(kIA32ApicBaseMsr, new_address | kIA32ApicBaseMsrEnable);
}

// --------------------------------
// Main controlling functions
// --------------------------------

void Enable();

FAST_CALL void WriteRegister(const u32 offset, const u32 value)
{
    TODO_WHEN_VMEM_WORKS
    WriteMemoryIo<u32>(
        reinterpret_cast<void *>(GetPhysicalAddress()),  // TODO : REPLACE WITH V ADDERSS
        offset, value
    );
}

FAST_CALL u32 ReadRegister(const u32 offset)
{
    TODO_WHEN_VMEM_WORKS
    return ReadMemoryIo<u32>(
        reinterpret_cast<void *>(GetPhysicalAddress()),  // TODO : REPLACE WITH V ADDERSS
        offset
    );
}
}  // namespace LocalApic
#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_LOCAL_APIC_HPP_
