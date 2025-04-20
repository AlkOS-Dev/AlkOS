#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_

#include <uacpi/acpi.h>
#include <extensions/bit_array.hpp>
#include <extensions/debug.hpp>
#include <extensions/types.hpp>

// ------------------------------
// Driver class
// ------------------------------

/**
 * HPET Driver - Manages High Precision Event Timer hardware
 * Used for high-resolution timing and as a potential system timer source
 */
class Hpet final
{
    public:
    // ------------------------------
    // Driver defines
    // ------------------------------

    /**
     * HPET registers memory offsets, RO - Read Only, RW - Read Write
     */

    static constexpr u32 kGeneralCapabilitiesRegRO =
        0x00;  // Contains timer capabilities and configuration
    static constexpr u32 kGeneralConfigurationRegRW = 0x10;  // Controls overall HPET functionality
    static constexpr u32 kGeneralInterruptStatusRegRW =
        0x20;  // Shows which timers have triggered interrupts
    static constexpr u32 kMainCounterValueRegRO = 0xF0;  // Current value of the main counter

    /**
     * Returns the memory offset for a timer's configuration register
     * @param timer_idx The timer index (0-based)
     */
    FAST_CALL constexpr u32 GetTimerConfigurationRegRW(const u32 timer_idx)
    {
        return 0x100 + (timer_idx * 0x20);
    }

    /**
     * Returns the memory offset for a timer's comparator value register
     * Used to set when this timer should trigger
     */
    FAST_CALL constexpr u32 GetTimerComparatorValueRegRW(const u32 timer_idx)
    {
        return 0x108 + (timer_idx * 0x20);
    }

    /**
     * Returns the memory offset for a timer's FSB route register
     * Used for Front Side Bus message-signaled interrupts
     */
    FAST_CALL constexpr u32 GetTimerFSBRouteRegRW(const u32 timer_idx)
    {
        return 0x110 + (timer_idx * 0x20);
    }

    // ------------------------------
    // Register structures
    // ------------------------------

    /**
     * General capabilities register - Provides hardware information about this HPET
     */
    struct PACK GeneralCapabilitiesAndIdReg {
        u8 revision_id;               // Hardware revision number
        u8 num_comparators : 5;       // Number of timers available (0-31)
        u8 is_64_bit_comparator : 1;  // Whether counters are 64-bit (1) or 32-bit (0)
        u8 reserved : 1;
        u8 is_legacy_replacement : 1;  // Can replace legacy 8254 PIT
        u16 vendor_id;                 // Hardware vendor identifier
        u32 clock_period;              // Timer period in femtoseconds
    };
    static_assert(sizeof(GeneralCapabilitiesAndIdReg) == 8);

    /**
     * Controls the overall HPET timer operation
     */
    struct PACK GeneralConfigurationReg {
        enum class EnableBit : u8 {
            kEnable  = 1,  // Start the main counter
            kDisable = 0,  // Stop the main counter
        };

        enum class LegacyReplacementBit : u8 {
            kEnable  = 1,  // Enable PIT replacement routing
            kDisable = 0,  // Disable PIT replacement routing
        };

        EnableBit enable : 1;
        LegacyReplacementBit legacy_replacement : 1;
        u64 reserved : 62;
    };
    static_assert(sizeof(GeneralConfigurationReg) == 8);

    /**
     * Shows which timers have triggered interrupts
     * Writing 1 to a bit clears the corresponding interrupt
     */
    struct PACK GeneralInterruptStatusReg {
        BitArray<32> interrupt_status;  // Bit set for each timer that triggered
        u32 reserved;
    };
    static_assert(sizeof(GeneralInterruptStatusReg) == 8);

    /**
     * Configuration for an individual timer
     */
    struct PACK TimerConfigurationReg {
        enum class InterruptType : u8 {
            kEdgeTriggered  = 0,  // Generate single pulse interrupt
            kLevelTriggered = 1,  // Generate level interrupt until cleared
        };

        enum class Enabled : u8 {
            kEnable  = 1,  // Timer can generate interrupts
            kDisable = 0,  // Timer won't generate interrupts
        };

        enum class TimerType : u8 {
            kOneShot  = 0,  // Timer triggers once and stops
            kPeriodic = 1,  // Timer triggers at regular intervals
        };

        enum class PeriodicSupported : u8 {
            kNotSupported = 0,  // Timer doesn't support periodic mode
            kSupported    = 1,  // Timer supports periodic mode
        };

        enum class Is64BitComparator : u8 {
            kNot64Bit = 0,  // Timer uses 32-bit comparator
            k64Bit    = 1,  // Timer uses 64-bit comparator
        };

        enum class Forced32Bit : u8 {
            kNotForced = 0,  // Use native width
            kForced    = 1,  // Force 32-bit mode for a 64-bit timer
        };

        enum class FsbRoute : u8 {
            kNotSet = 0,  // Use normal interrupt delivery
            kSet    = 1,  // Use FSB message delivery
        };

        enum class FsbSupported : u8 {
            kNotSupported = 0,  // Timer doesn't support FSB routing
            kSupported    = 1,  // Timer supports FSB routing
        };

        u8 reserved1 : 1;
        InterruptType interrupt_type : 1;
        Enabled enabled : 1;
        TimerType timer_type : 1;
        const PeriodicSupported periodic_supported : 1;
        const Is64BitComparator is_64_bit_comparator : 1;
        u8 allow_direct_periodic_access : 1;  // Enables direct writes to comparator for periodic
                                              // mode
        u8 reserved2 : 1;
        Forced32Bit forced_32_bit : 1;
        u8 vector : 5;  // Interrupt vector to trigger
        FsbRoute fsb_route : 1;
        const FsbSupported fsb_supported : 1;
        u16 reserved3;
        const BitArray<32> route_capabilities;  // Which interrupt routes are available
    };
    static_assert(sizeof(TimerConfigurationReg) == 8);

    // ------------------------------
    // Class creation
    // ------------------------------

    /**
     * Initialize HPET driver from ACPI table
     * @param table Pointer to the ACPI HPET description table
     */
    explicit Hpet(acpi_hpet* table);

    // ------------------------------
    // Class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    acpi_gas address_;           // Memory-mapped register address information
    u8 num_comparators_;         // Number of timer comparators available
    bool is_comparator_64_bit_;  // Whether comparators are 64-bit capable
    u16 ticks_;                  // Timer frequency information
};

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_
