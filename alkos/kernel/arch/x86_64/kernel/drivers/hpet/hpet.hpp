#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_

#include <uacpi/acpi.h>
#include <extensions/bit_array.hpp>
#include <extensions/debug.hpp>
#include <extensions/types.hpp>

// ------------------------------
// Driver class
// ------------------------------

class Hpet final
{
    public:
    // ------------------------------
    // Driver defines
    // ------------------------------

    /**
     * HPET registers, RO - Read Only, RW - Read Write
     */

    static constexpr u32 kGeneralCapabilitiesRegRO    = 0x00;
    static constexpr u32 kGeneralConfigurationRegRW   = 0x10;
    static constexpr u32 kGeneralInterruptStatusRegRW = 0x20;
    static constexpr u32 kMainCounterValueRegRO       = 0xF0;

    FAST_CALL constexpr u32 GetTimerConfigurationRegRW(const u32 timer_idx)
    {
        return 0x100 + (timer_idx * 0x20);
    }

    FAST_CALL constexpr u32 GetTimerComparatorValueRegRW(const u32 timer_idx)
    {
        return 0x108 + (timer_idx * 0x20);
    }

    FAST_CALL constexpr u32 GetTimerFSBRouteRegRW(const u32 timer_idx)
    {
        return 0x110 + (timer_idx * 0x20);
    }

    // ------------------------------
    // Register structures
    // ------------------------------

    /**
     * General HPET registers
     */
    struct PACK GeneralCapabilitiesAndIdReg {
        u8 revision_id;
        u8 num_comparators : 5;
        u8 is_64_bit_comparator : 1;
        u8 reserved : 1;
        u8 is_legacy_replacement : 1;
        u16 vendor_id;
        u32 clock_period;
    };
    static_assert(sizeof(GeneralCapabilitiesAndIdReg) == 8);

    struct PACK GeneralConfigurationReg {
        enum class EnableBit : u8 {
            kEnable  = 1,
            kDisable = 0,
        };

        enum class LegacyReplacementBit : u8 {
            kEnable  = 1,
            kDisable = 0,
        };

        EnableBit enable : 1;
        LegacyReplacementBit legacy_replacement : 1;
        u64 reserved : 62;
    };
    static_assert(sizeof(GeneralConfigurationReg) == 8);

    struct PACK GeneralInterruptStatusReg {
        BitArray<32> interrupt_status;
        u32 reserved;
    };
    static_assert(sizeof(GeneralInterruptStatusReg) == 8);

    /**
     * Timer specific registers
     */

    struct PACK TimerConfigurationReg {
        enum class InterruptType : u8 {
            kEdgeTriggered  = 0,
            kLevelTriggered = 1,
        };

        enum class Enabled : u8 {
            kEnable  = 1,
            kDisable = 0,
        };

        enum class TimerType : u8 {
            kOneShot  = 0,
            kPeriodic = 1,
        };

        enum class PeriodicSupported : u8 {
            kNotSupported = 0,
            kSupported    = 1,
        };

        enum class Is64BitComparator : u8 {
            kNot64Bit = 0,
            k64Bit    = 1,
        };

        enum class Forced32Bit : u8 {
            kNotForced = 0,
            kForced    = 1,
        };

        enum class FsbRoute : u8 {
            kNotSet = 0,
            kSet    = 1,
        };

        enum class FsbSupported : u8 {
            kNotSupported = 0,
            kSupported    = 1,
        };

        u8 reserved1 : 1;
        InterruptType interrupt_type : 1;
        Enabled enabled : 1;
        TimerType timer_type : 1;
        const PeriodicSupported periodic_supported : 1;
        const Is64BitComparator is_64_bit_comparator : 1;
        u8 allow_direct_periodic_access : 1;
        u8 reserved2 : 1;
        Forced32Bit forced_32_bit : 1;
        u8 vector : 5;
        FsbRoute fsb_route : 1;
        const FsbSupported fsb_supported : 1;
        u16 reserved3;
        const BitArray<32> route_capabilities;
    };
    static_assert(sizeof(TimerConfigurationReg) == 8);

    // ------------------------------
    // Class creation
    // ------------------------------

    explicit Hpet(acpi_hpet* table);

    // ------------------------------
    // Class methods
    // ------------------------------

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    acpi_gas address_;
    u8 num_comparators_;
    bool is_comparator_64_bit_;
    u16 ticks_;
};

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_
