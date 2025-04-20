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
 * TODO: SUPPORT 2x APIC
 */

/**
 * @file local_apic.hpp
 * @brief Local APIC (Advanced Programmable Interrupt Controller) driver for x86_64 architecture
 *
 * The Local APIC is responsible for managing CPU-specific interrupts, including:
 * - Receiving and dispatching external interrupts to the CPU core
 * - Generating and handling inter-processor interrupts (IPIs)
 * - Managing timer interrupts with configurable frequencies
 * - Handling error, thermal, and performance monitoring interrupts
 *
 * Each processor in a multiprocessor system has its own Local APIC. The Local APIC
 * is memory-mapped and communicates with the CPU via memory-mapped I/O (MMIO).
 *
 * Reference: Intel 64 and IA-32 Architectures Software Developer's Manual
 * - Volume 3A: System Programming Guide, Part 1
 * - Chapter 10: Advanced Programmable Interrupt Controller (APIC)
 * - Sections 10.4 through 10.14 detail the Local APIC functionality and register interface
 *
 * @see https://www.intel.com/content/www/us/3en/developer/articles/technical/intel-sdm.html
 */
namespace LocalApic
{
// ------------------------------
// Architecture Defines and Constants
// ------------------------------

/**
 * @brief CPU feature detection and MSR access constants
 *
 * These constants are used to detect Local APIC support and control its base address
 * through Model-Specific Registers (MSRs).
 */
static constexpr u32 kEdxAcpiFlag = 1 << 9;  ///< CPUID[1].EDX bit indicating APIC support
static constexpr u32 kIA32ApicBaseMsr =
    0x1B;  ///< MSR number for APIC base address (IA32_APIC_BASE)
static constexpr u64 kIA32ApicBaseMsrEnable =
    0x800;  ///< Bit 11 in IA32_APIC_BASE to enable Local APIC

/**
 * @brief Local APIC Memory-Mapped Register Offsets
 *
 * All registers are 32 bits wide but are aligned on 16-byte boundaries (4 bytes of data, 12 bytes
 * reserved). Access type: RW = Read/Write, RO = Read Only, WO = Write Only
 *
 * Note: When accessing these registers, add the offset to the base MMIO address of the Local APIC.
 */

/* 0x000-0x010 - RESERVED */
static constexpr u32 kIdRegRW = 0x020;  ///< [RW] Local APIC ID Register - CPU identification
static constexpr u32 kVersionRegRO =
    0x030;  ///< [RO] Version Register - APIC version and max LVT entries
/* 0x040 - 0x070 - RESERVED */
static constexpr u32 kTaskPriorityRegRW =
    0x080;  ///< [RW] Task Priority Register (TPR) - Sets interrupt acceptance threshold
static constexpr u32 kArbitrationPriorityRegRO =
    0x090;  ///< [RO] Arbitration Priority Register (APR) - Used in MP systems
static constexpr u32 kProcessorPriorityRegRO =
    0x0A0;  ///< [RO] Processor Priority Register (PPR) - Current priority after applying TPR
static constexpr u32 kEndOfInterruptRegWO =
    0x0B0;  ///< [WO] End Of Interrupt Register (EOI) - Signals completion of interrupt handling
static constexpr u32 kRemoteReadRegRO =
    0x0C0;  ///< [RO] Remote Read Register (RRD) - Used for inter-APIC communication
static constexpr u32 kLogicalDestinationRegRW =
    0x0D0;  ///< [RW] Logical Destination Register - Sets logical APIC ID for IPIs
static constexpr u32 kDestinationFormatRegRW =
    0x0E0;  ///< [RW] Destination Format Register - Controls logical addressing mode
static constexpr u32 kSpuriousInterruptRegRW =
    0x0F0;  ///< [RW] Spurious Interrupt Vector Register - Global APIC enable and spurious vector

/**
 * @brief Interrupt Status Registers
 *
 * These register blocks track interrupt state across 256 possible vectors,
 * with each bit in the register corresponding to a specific vector.
 */
/* 0x100 - 0x170 - In-Service Register (ISR) - Tracks interrupts currently being serviced */
/* 0x180 - 0x1F0 - Trigger Mode Register (TMR) - Tracks edge/level triggered interrupts */
/* 0x200 - 0x270 - Interrupt Request Register (IRR) - Tracks pending interrupts */

static constexpr u32 kErrorStatusRegRO =
    0x280;  ///< [RO] Error Status Register - Records APIC error conditions
/* 0x290 - 0x2E0 - RESERVED */

/**
 * @brief Local Vector Table (LVT) and Interrupt Control Registers
 *
 * These registers control various local interrupt sources and how the
 * Local APIC handles specific interrupt events.
 */
static constexpr u32 kLvtCmciRegRW =
    0x2F0;  ///< [RW] LVT CMCI Register - Corrected Machine Check Interrupt control
static constexpr u32 kInterruptCommandLowRegRW =
    0x300;  ///< [RW] Interrupt Command Register (Low) - IPI control (low 32 bits)
static constexpr u32 kInterruptCommandHighRegRW =
    0x310;  ///< [RW] Interrupt Command Register (High) - IPI control (high 32 bits)
static constexpr u32 kLvtTimerRegRW =
    0x320;  ///< [RW] LVT Timer Register - Local APIC timer configuration
static constexpr u32 kLvtThermalSensorRegRW =
    0x330;  ///< [RW] LVT Thermal Sensor Register - Thermal sensor interrupt control
static constexpr u32 kLvtPerformanceCounterRegRW =
    0x340;  ///< [RW] LVT Performance Counter Register - PMC interrupt control
static constexpr u32 kLvtLint0RegRW = 0x350;  ///< [RW] LVT e
static constexpr u32 kLvtLint1RegRW =
    0x360;  ///< [RW] LVT LINT1 Register - Local interrupt 1 configuration
static constexpr u32 kLvtErrorRegRW =
    0x370;  ///< [RW] LVT Error Register - APIC error interrupt configuration

/**
 * @brief Timer Control Registers
 *
 * The Local APIC includes a programmable timer that can generate
 * periodic or one-shot interrupts for the local processor.
 */
static constexpr u32 kInitialCountRegRW =
    0x380;  ///< [RW] Initial Count Register - Sets timer countdown start value
static constexpr u32 kCurrentCountRegRO =
    0x390;  ///< [RO] Current Count Register - Current timer value (read-only)
/* 0x3A0 - 0x3D0 - RESERVED */
static constexpr u32 kDivideConfigRegRW =
    0x3E0;  ///< [RW] Divide Configuration Register - Timer frequency divider
/* 0x3F0 - RESERVED */

/**
 * @brief End-of-Interrupt (EOI) constant
 *
 * Writing this value to the EOI register acknowledges completion
 * of the current interrupt service routine.
 */
static constexpr u32 kEOISignal = 0;

struct LocalVectorTableRegister {
    enum class DeliveryMode : u8 {
        kFixed   = 0b000,  ///< Normal interrupt delivery to vector specified
        kSMI     = 0b010,  ///< System Management Interrupt (enters SMM)
        kNMI     = 0b100,  ///< Non-Maskable Interrupt (cannot be masked)
        kINIT    = 0b101,  ///< INIT signal (processor reset without memory reset)
        kExtINIT = 0b110,  ///< External INIT
    };

    enum class DeliveryStatus : u8 {
        kIdle    = 0,
        kPending = 1,
    };

    enum class Mask {
        kEnabled  = 0,
        kDisabled = 1,
    };

    enum class Polarity : u8 {
        kActiveHigh = 0,
        kActiveLow  = 1,
    };

    enum class TriggerMode : u8 {
        kEdgeTriggered  = 0,
        kLevelTriggered = 1,
    };

    u32 vector : 8;
    DeliveryMode delivery_mode : 3;
    u32 reserved1 : 1;
    DeliveryStatus delivery_status : 1;
    Polarity polarity : 1;
    u32 remote_irr : 1;
    TriggerMode trigger_mode : 1;
    Mask mask : 1;
    u32 reserved2 : 15;
};
static_assert(sizeof(LocalVectorTableRegister) == 4);

struct LocalVectorTableTimerRegister {
    enum class DeliveryStatus : u8 {
        kIdle    = 0,
        kPending = 1,
    };

    enum class Mask {
        kEnabled  = 0,
        kDisabled = 1,
    };

    enum class TimerMode : u8 {
        kOneShot     = 0b00,
        kPeriodic    = 0b01,
        kTSCDeadline = 0b10,
    };

    u32 vector : 8;
    u32 reserved1 : 4;
    DeliveryStatus delivery_status : 1;
    u32 reserved2 : 3;
    Mask mask : 1;
    TimerMode timer_mode : 2;
};
static_assert(sizeof(LocalVectorTableTimerRegister) == 4);

/**
 * @brief Spurious Interrupt Vector Register Structure
 *
 * Controls global APIC enable/disable and defines the vector
 * for spurious interrupts (false or unwanted interrupts).
 */
struct SpuriousInterruptRegister {
    enum class State {
        kDisabled = 0,
        kEnabled  = 1,
    };

    u32 vector : 8;  ///< Vector to deliver for spurious interrupts
    State enabled : 1;
    u32 reserved1 : 3;
    u32 no_eoi_broadcast : 1;  ///< 1 = Suppress EOI broadcasts in x2APIC mode
    u32 reserved2 : 19;
};
static_assert(sizeof(SpuriousInterruptRegister) == 4);

/**
 * @brief Interrupt Command Register Structure
 *
 * Used to send Inter-Processor Interrupts (IPIs) to other processors
 * in the system. This structure represents the lower 32 bits of the ICR.
 * The upper 32 bits contain the destination processor ID.
 */
struct InterruptCommandRegister {
    enum class DeliveryMode : u8 {
        kFixed         = 0b000,  ///< Normal interrupt delivery to vector specified
        kLowerPriority = 0b001,  ///< Lowest-priority delivery (for load balancing)
        kSMI           = 0b010,  ///< System Management Interrupt (enters SMM)
        kNMI           = 0b100,  ///< Non-Maskable Interrupt (cannot be masked)
        kINIT          = 0b101,  ///< INIT signal (processor reset without memory reset)
        kSIPI          = 0b110,  ///< Startup IPI (used during AP initialization)
    };

    enum class DestinationMode : u8 {
        kPhysical = 0,
        kLogical  = 1,
    };

    enum class InitType : u8 {
        kNormal   = 0b01,
        kDeAssert = 0b10,  ///< INIT De-Assert (used to complete INIT sequence)
    };

    enum class DeliveryStatus : u8 {
        kIdle    = 0,
        kPending = 1,
    };

    enum class DestinationType : u8 {
        kNormal                  = 0,  ///< Use destination ID in ICR high register
        kNotifyYourself          = 1,  ///< Send IPI to self only
        kBroadcast               = 2,  ///< Send IPI to all processors including self
        kBroadcastExceptYourself = 3   ///< Send IPI to all processors except self
    };

    u32 vector : 8;                        ///< Interrupt vector (0-255) to deliver to destination
    DeliveryMode delivery_mode : 3;        ///< How the interrupt should be handled by destination
    DestinationMode destination_mode : 1;  ///< 0 = Physical ID, 1 = Logical ID
    DeliveryStatus delivery_status : 1;    ///< Set by hardware when IPI is pending (read-only)
    u32 reserved1 : 1;                     ///< Reserved bit
    InitType init_type : 2;                ///< Additional flags for INIT IPIs
    DestinationType destination_type : 2;  ///< Destination shorthand for common IPI targets
    u32 reserved2 : 12;                    ///< Reserved bits
};
static_assert(sizeof(InterruptCommandRegister) == 4);

// ------------------------------
// Utility Functions
// ------------------------------

/**
 * @brief Checks if Local APIC is supported by the CPU
 *
 * Uses CPUID to determine if the processor supports the Local APIC.
 *
 * @return true if Local APIC is supported, false otherwise
 */
NODISCARD FAST_CALL bool IsSupported()
{
    unsigned int edx;
    unsigned int unused;

    __get_cpuid(1, &unused, &unused, &unused, &edx);
    return edx & kEdxAcpiFlag;
}

/**
 * @brief Gets the physical base address of the Local APIC
 *
 * Reads the IA32_APIC_BASE MSR to determine the current memory-mapped
 * address of the Local APIC registers.
 *
 * @return 4K-aligned physical address of the Local APIC MMIO region
 */
NODISCARD FAST_CALL u64 GetPhysicalAddress()
{
    /* Return 4k page aligned address, removing the lower 12 bits (flags) */
    return CpuGetMSR(kIA32ApicBaseMsr) & ~kBitMaskRight<u64, 12>;
}

/**
 * @brief Sets the physical base address of the Local APIC
 *
 * Updates the IA32_APIC_BASE MSR with a new physical address for the
 * Local APIC MMIO region, while also ensuring the APIC remains enabled.
 *
 * @param new_address New 4K-aligned physical address for the Local APIC
 */
FAST_CALL void SetPhysicalAddress(const u64 new_address)
{
    ASSERT_TRUE(IsAligned(new_address, 12), "Local APIC address is not aligned to 4k page!");
    CpuSetMSR(kIA32ApicBaseMsr, new_address | kIA32ApicBaseMsrEnable);
}

// --------------------------------
// Main Control Functions
// --------------------------------

/**
 * @brief Enables the Local APIC
 *
 * Initializes and enables the Local APIC by:
 * 1. Setting the enable bit in the IA32_APIC_BASE MSR
 * 2. Configuring the Spurious Interrupt Vector Register
 * 3. Setting up default values for critical registers
 */
void Enable();

/**
 * @brief Writes a value to a Local APIC register
 *
 * Performs memory-mapped I/O to write to the specified Local APIC register.
 *
 * @param offset Register offset from the Local APIC base address
 * @param value 32-bit value to write to the register
 */

template <class InputT>
FAST_CALL void WriteRegister(const u32 offset, const InputT value)
{
    TODO_WHEN_VMEM_WORKS
    WriteMemoryIo<u32>(
        reinterpret_cast<byte *>(GetPhysicalAddress()),  // TODO : REPLACE WITH VIRTUAL ADDRESS
        offset, value
    );
}

/**
 * @brief Reads a value from a Local APIC register
 *
 * Performs memory-mapped I/O to read from the specified Local APIC register.
 *
 * @param offset Register offset from the Local APIC base address
 * @return 32-bit value read from the register
 */
template <class RetT = u32>
FAST_CALL RetT ReadRegister(const u32 offset)
{
    TODO_WHEN_VMEM_WORKS
    return ReadMemoryIo<u32, RetT>(
        reinterpret_cast<byte *>(GetPhysicalAddress()),  // TODO : REPLACE WITH VIRTUAL ADDRESS
        offset
    );
}

/**
 * @brief Signals End-Of-Interrupt to the Local APIC
 *
 * This function must be called at the end of each interrupt service routine
 * to inform the Local APIC that interrupt processing is complete. This allows
 * the Local APIC to dequeue the interrupt from the ISR register and accept
 * new interrupts of the same or lower priority.
 */
FAST_CALL void SendEOI() { WriteRegister(kEndOfInterruptRegWO, kEOISignal); }

FAST_CALL u8 GetCoreId() { return ReadRegister(kIdRegRW) >> 24; }

}  // namespace LocalApic
#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_APIC_LOCAL_APIC_HPP_
