#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_

#include <uacpi/acpi.h>
#include <extensions/array.hpp>
#include <extensions/bit_array.hpp>
#include <extensions/debug.hpp>
#include <extensions/types.hpp>
#include "memory_io.hpp"
#include "todo.hpp"

// ------------------------------
// Driver class
// ------------------------------

/**
 * HPET Driver - Manages High Precision Event Timer hardware
 * Used for high-resolution timing and as a potential system timer source
 *
 * Refer to:
 * https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf
 * and: https://wiki.osdev.org/HPET
 *
 * TODO: Things to be done:
 * - make real use of the timers
 * - state is not restored after sleep operation
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
    static constexpr u32 kMainCounterValueRegRO       = 0xF0;  // Current value of the main counter
    static constexpr u32 kTimerConfigurationRegBaseRW = 0x100;
    static constexpr u32 kTimerComparatorValueRegRW   = 0x108;
    static constexpr u32 kTimerFSBInterruptRouteRegRW = 0x110;
    static constexpr u32 kTimerAllRegSize             = 0x20;
    static constexpr u32 kComparatorMaxIrqMap         = 32;
    static constexpr u32 kMaxComparators              = 32;

    /**
     * Returns the memory offset for a timer's configuration register
     * @param timer_idx The timer index (0-based)
     */
    FAST_CALL constexpr u32 GetTimerConfigurationRegRW(const u32 timer_idx)
    {
        return kTimerConfigurationRegBaseRW + (timer_idx * kTimerAllRegSize);
    }

    /**
     * Returns the memory offset for a timer's comparator value register
     * Used to set when this timer should trigger
     */
    FAST_CALL constexpr u32 GetTimerComparatorValueRegRW(const u32 timer_idx)
    {
        return kTimerComparatorValueRegRW + (timer_idx * kTimerAllRegSize);
    }

    /**
     * Returns the memory offset for a timer's FSB route register
     * Used for Front Side Bus message-signaled interrupts
     */
    FAST_CALL constexpr u32 GetTimerFSBRouteRegRW(const u32 timer_idx)
    {
        return kTimerFSBInterruptRouteRegRW + (timer_idx * kTimerAllRegSize);
    }

    // ------------------------------
    // Register structures
    // ------------------------------

    /**
     * General capabilities register - Provides hardware information about this HPET
     */
    struct PACK GeneralCapabilitiesAndIdReg {
        enum class TimerType : u8 {
            kLegacyReplacement = 1,  // HPET can replace legacy 8254 PIT
            kGeneralPurpose    = 0,  // General purpose timer
        };

        enum class TimerSize : u8 {
            k32Bit = 0,  // 32-bit timer
            k64Bit = 1,  // 64-bit timer
        };

        u8 revision_id;            // Hardware revision number
        u8 num_comparators : 5;    // Number of timers available (0-31)
        TimerSize timer_size : 1;  // Whether counters are 64-bit (1) or 32-bit (0)
        u8 : 1;                    // Reserved
        TimerType timer_type : 1;  // Can replace legacy 8254 PIT
        u16 vendor_id;             // Hardware vendor identifier
        u32 clock_period;          // Timer period in femtoseconds
    };
    static_assert(sizeof(GeneralCapabilitiesAndIdReg) == 8);

    /**
     * Controls the overall HPET timer operation
     */
    struct PACK GeneralConfigurationReg {
        enum class LegacyReplacementBit : u8 {
            kEnable  = 1,  // Enable PIT replacement routing
            kDisable = 0,  // Disable PIT replacement routing
        };

        bool enable : 1;
        LegacyReplacementBit legacy_replacement : 1;
        u64 : 62;  // Reserved
    };
    static_assert(sizeof(GeneralConfigurationReg) == 8);

    /**
     * Shows which timers have triggered interrupts
     * Writing 1 to a bit clears the corresponding interrupt
     */
    struct PACK GeneralInterruptStatusReg {
        BitArray<kMaxComparators> interrupt_status;  // Bit set for each timer that triggered
        u32 : 32;                                    // Reserved
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

        u8 : 1;  // Reserved
        InterruptType interrupt_type : 1;
        Enabled enabled : 1;
        TimerType timer_type : 1;
        const PeriodicSupported periodic_supported : 1;
        const Is64BitComparator is_64_bit_comparator : 1;
        bool periodic_mem_access : 1;
        u8 : 1;  // Reserved
        Forced32Bit forced_32_bit : 1;
        u8 vector : 5;  // Interrupt vector to trigger
        FsbRoute fsb_route : 1;
        const FsbSupported fsb_supported : 1;
        u16 : 16;                                            // Reserved
        const BitArray<kMaxComparators> route_capabilities;  // Which interrupt routes are available
    };
    static_assert(sizeof(TimerConfigurationReg) == 8);

    // ------------------------------
    // Class creation
    // ------------------------------

    /**
     * Initialize HPET driver from ACPI table
     * @param table Pointer to the ACPI HPET description table
     */
    explicit Hpet(acpi_hpet *table);

    // ------------------------------
    // Utilities
    // ------------------------------

    NODISCARD FORCE_INLINE_F u64 GetPhysicalAddress() const { return address_.address; }

    template <class InputT>
    FORCE_INLINE_F void WriteRegister(const u32 offset, const InputT value)
    {
        TODO_WHEN_VMEM_WORKS
        // TODO : REPLACE WITH VIRTUAL ADDRESS
        WriteMemoryIo<u64>(reinterpret_cast<byte *>(GetPhysicalAddress()), offset, value);
    }

    template <class RetT>
    FORCE_INLINE_F RetT ReadRegister(const u32 offset)
    {
        TODO_WHEN_VMEM_WORKS
        // TODO : REPLACE WITH VIRTUAL ADDRESS
        return ReadMemoryIo<u64, RetT>(reinterpret_cast<byte *>(GetPhysicalAddress()), offset);
    }

    // ------------------------------
    // Class methods
    // ------------------------------

    TODO_WHEN_TIMER_INFRA_DONE
    // Temporary function replacing PIT
    void Setup();

    NODISCARD FORCE_INLINE_F bool IsTimerSupportingPeriodic(const u32 timer_idx) const
    {
        ASSERT_LT(timer_idx, num_comparators_);
        return comparators_periodic_supported_.Get(timer_idx);
    }

    NODISCARD FORCE_INLINE_F bool IsTimer64Bit(const u32 timer_idx) const
    {
        ASSERT_LT(timer_idx, num_comparators_);
        return comparators_64bit_supported_.Get(timer_idx);
    }

    NODISCARD FORCE_INLINE_F bool IsIrqSupportedOnTimer(
        const u32 timer_idx, const u32 irq_map
    ) const
    {
        ASSERT_LT(timer_idx, num_comparators_);
        return comparators_allowed_irqs_[timer_idx].Get(irq_map);
    }

    NODISCARD FORCE_INLINE_F u64 ReadMainCounter()
    {
        return ReadRegister<u64>(kMainCounterValueRegRO);
    }

    FORCE_INLINE_F void SetupOneShotTimer(
        const u32 timer_idx, const u64 time_femto_seconds, const u32 irq_map
    )
    {
        ASSERT_LT(timer_idx, num_comparators_);
        ASSERT_LT(irq_map, kComparatorMaxIrqMap);

        auto timer_conf =
            ReadRegister<TimerConfigurationReg>(GetTimerConfigurationRegRW(timer_idx));
        ASSERT_TRUE(
            IsIrqSupportedOnTimer(timer_idx, irq_map),
            "Given irq map is not allowed for this HPET timer"
        );

        timer_conf.enabled    = TimerConfigurationReg::Enabled::kEnable;
        timer_conf.timer_type = TimerConfigurationReg::TimerType::kOneShot;
        timer_conf.vector     = irq_map;

        WriteRegister(GetTimerConfigurationRegRW(timer_idx), timer_conf);
        WriteRegister(
            GetTimerComparatorValueRegRW(timer_idx),
            ReadMainCounter() + AdjustTimeToHpetCapabilities(time_femto_seconds)
        );
    }

    FORCE_INLINE_F void SetupPeriodicTimer(
        const u32 timer_idx, const u64 period_femto_seconds, const u32 irq_map
    )
    {
        ASSERT_LT(timer_idx, num_comparators_);
        ASSERT_LT(irq_map, kComparatorMaxIrqMap);
        ASSERT_TRUE(IsTimerSupportingPeriodic(timer_idx));

        auto timer_conf =
            ReadRegister<TimerConfigurationReg>(GetTimerConfigurationRegRW(timer_idx));
        ASSERT_TRUE(
            IsIrqSupportedOnTimer(timer_idx, irq_map),
            "Given irq map is not allowed for this HPET timer"
        );

        timer_conf.enabled             = TimerConfigurationReg::Enabled::kEnable;
        timer_conf.timer_type          = TimerConfigurationReg::TimerType::kPeriodic;
        timer_conf.periodic_mem_access = true;
        timer_conf.vector              = irq_map;

        WriteRegister(GetTimerConfigurationRegRW(timer_idx), timer_conf);

        /* First periodic shot */
        WriteRegister(
            GetTimerComparatorValueRegRW(timer_idx),
            ReadMainCounter() + AdjustTimeToHpetCapabilities(period_femto_seconds)
        );

        /* Periodic increment */
        WriteRegister(
            GetTimerComparatorValueRegRW(timer_idx),
            AdjustTimeToHpetCapabilities(period_femto_seconds)
        );
    }

    NODISCARD FORCE_INLINE_F bool IsLegacyReplacementSupported() const
    {
        return is_legacy_mode_available_;
    }

    FORCE_INLINE_F void SetupStandardMapping()
    {
        auto conf_reg = ReadRegister<GeneralConfigurationReg>(kGeneralConfigurationRegRW);
        conf_reg.legacy_replacement = GeneralConfigurationReg::LegacyReplacementBit::kDisable;

        WriteRegister(kGeneralConfigurationRegRW, conf_reg);
        is_legacy_mode_ = false;
    }

    FORCE_INLINE_F void SetupLegacyMapping()
    {
        ASSERT_TRUE(IsLegacyReplacementSupported());

        auto conf_reg = ReadRegister<GeneralConfigurationReg>(kGeneralConfigurationRegRW);
        conf_reg.legacy_replacement = GeneralConfigurationReg::LegacyReplacementBit::kEnable;

        WriteRegister(kGeneralConfigurationRegRW, conf_reg);
        is_legacy_mode_ = true;
    }

    NODISCARD FORCE_INLINE_F bool IsLegacyModeEnabled() const { return is_legacy_mode_; }

    FORCE_INLINE_F void StartMainCounter()
    {
        auto conf_reg   = ReadRegister<GeneralConfigurationReg>(kGeneralConfigurationRegRW);
        conf_reg.enable = true;

        WriteRegister(kGeneralConfigurationRegRW, conf_reg);
        is_main_counter_enabled_ = true;
    }

    FORCE_INLINE_F void StopMainCounter()
    {
        auto conf_reg   = ReadRegister<GeneralConfigurationReg>(kGeneralConfigurationRegRW);
        conf_reg.enable = false;

        WriteRegister(kGeneralConfigurationRegRW, conf_reg);
        is_main_counter_enabled_ = false;
    }

    NODISCARD FORCE_INLINE_F bool IsMainCounterEnabled() const { return is_main_counter_enabled_; }

    // ------------------------------
    // Private methods
    // ------------------------------

    private:
    NODISCARD u64 AdjustTimeToHpetCapabilities(u64 time_femto) const;

    // ------------------------------
    // Class fields
    // ------------------------------

    /* comparators gathered data */
    BitArray<kMaxComparators> comparators_64bit_supported_{};
    BitArray<kMaxComparators> comparators_periodic_supported_{};
    std::array<BitArray<kComparatorMaxIrqMap>, kMaxComparators> comparators_allowed_irqs_{};

    /* General hpet information */
    acpi_gas address_{};    // Memory-mapped register address information
    u8 num_comparators_{};  // Number of timer comparators available
    u32 clock_period_{};    // Amount of femto-seconds needed for HPET to update the counter

    bool is_legacy_mode_available_{};
    bool is_counter_32_bit_{};
    bool is_legacy_mode_{};
    bool is_main_counter_enabled_{};
};

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_DRIVERS_HPET_HPET_HPP_
