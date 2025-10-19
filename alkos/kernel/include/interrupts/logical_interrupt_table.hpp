#ifndef ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_HPP_
#define ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_HPP_

#include <assert.h>
#include <extensions/bit.hpp>
#include <extensions/cstddef.hpp>
#include <extensions/template_lib.hpp>
#include <extensions/type_traits.hpp>

namespace intr
{
enum class InterruptType : uint8_t {
    kException         = 0,
    kHardwareException = 1,
    kSoftwareException = 2,
};

template <
    std::size_t kNumExceptions, std::size_t kNumHardwareExceptions,
    std::size_t kNumSoftwareExceptions>
class LogicalInterruptTable
{
    static constexpr u16 kUnmappedIrq = kFullMask<u16>;
    static_assert(
        kNumExceptions < kUnmappedIrq, "Maximal allowed number of exception handlers exceeded"
    );
    static_assert(
        kNumHardwareExceptions < kUnmappedIrq,
        "Maximal allowed number of hardware interrupts handlers exceeded"
    );
    static_assert(
        kNumSoftwareExceptions < kUnmappedIrq,
        "Maximal allowed number of software interrupts handlers exceeded"
    );

    // ------------------------------
    // class types
    // ------------------------------

    public:
    template <InterruptType kInterruptType>
    struct InterruptHandlerEntry {
        /* Interrupt handler */
        using InterruptHandler = void (*)(InterruptHandlerEntry &entry);
        using InterruptHandlerException =
            void (*)(InterruptHandlerEntry &entry, hal::ExceptionData *data);
        using HandlerType = std::conditional_t<
            kInterruptType == InterruptType::kException, InterruptHandlerException,
            InterruptHandler>;

        /* Interrupt driver */
        struct InterruptDriver {
            struct callbacks {
                void (*ack)(InterruptHandlerEntry &);
            };

            callbacks cbs{};
            const char *name{};
            void *data{};
        };

        struct HandlerData {
            HandlerType handler{};
            void *data{};
        };

        HandlerData handler_data{};
        u16 logical_irq{};
        u64 hardware_irq{};
        template_lib::OptionalField<
            kInterruptType == InterruptType::kHardwareException, InterruptDriver>
            driver{};
    };

    template <InterruptType kInterruptType>
    using HandlerData = typename InterruptHandlerEntry<kInterruptType>::HandlerData;

    template <InterruptType kInterruptType>
    using HandlerType = typename InterruptHandlerEntry<kInterruptType>::HandlerType;

    using InterruptDriver =
        typename InterruptHandlerEntry<InterruptType::kHardwareException>::InterruptDriver;

    // ------------------------------
    // Class creation
    // ------------------------------

    LogicalInterruptTable();

    // ------------------------------
    // Class interaction
    // ------------------------------

    FORCE_INLINE_F void HandleInterrupt(const u16 lirq, hal::ExceptionData *data)
    {
        ASSERT_LT(lirq, GetTableSize_<InterruptType::kException>());
        ASSERT_FALSE(IsUnmapped_<InterruptType::kException>(lirq));
        auto &entry = GetTable_<InterruptType::kException>()[lirq];

        /* Exception MUST be handled */
        (*entry.handler_data.handler)(entry, data);
    }

    template <InterruptType kInterruptType>
        requires(kInterruptType != InterruptType::kException)
    FORCE_INLINE_F void HandleInterrupt(const u16 lirq)
    {
        ASSERT_LT(lirq, GetTableSize_<kInterruptType>());
        ASSERT_FALSE(IsUnmapped_<kInterruptType>(lirq));
        auto &entry = GetTable_<kInterruptType>()[lirq];

        if (entry.handler_data.handler) {
            (*entry.handler_data.handler)(entry);
        }

        if constexpr (kInterruptType == InterruptType::kHardwareException) {
            ASSERT_NOT_NULL(entry.driver, "Interrupt driver is not installed!");
            entry.driver.cbs.ack(entry);
        }
    }

    template <InterruptType kInterruptType>
    FORCE_INLINE_F void InstallInterruptHandler(
        const u16 lirq, const HandlerData<kInterruptType> &handler
    )
    {
        ASSERT_LT(lirq, GetTableSize_<kInterruptType>());
        GetTable_<kInterruptType>()[lirq].handler_data = handler;
    }

    template <InterruptType kInterruptType>
    FORCE_INLINE_F void MapLogicalInterruptToHw(const u16 lirq, const u64 hardware_irq)
    {
        ASSERT_LT(lirq, GetTableSize_<kInterruptType>());
        GetTable_<kInterruptType>()[lirq].hardware_irq = hardware_irq;
    }

    FORCE_INLINE_F void InstallInterruptDriver(const u16 lirq, InterruptDriver *driver)
    {
        ASSERT_LT(lirq, GetTableSize_<InterruptType::kHardwareException>());
        GetTable_<InterruptType::kHardwareException>()[lirq].driver = driver;
    }

    // ------------------------------
    // Implementation
    // ------------------------------

    private:
    template <InterruptType kInterruptType>
    FORCE_INLINE_F NODISCARD InterruptHandlerEntry<kInterruptType> *GetTable_()
    {
        if constexpr (kInterruptType == InterruptType::kException) {
            return exception_table_;
        } else if constexpr (kInterruptType == InterruptType::kHardwareException) {
            return hardware_exception_table_;
        } else if constexpr (kInterruptType == InterruptType::kSoftwareException) {
            return software_exception_table_;
        } else {
            R_FAIL_ALWAYS("Invalid type provided");
        }
    }

    template <InterruptType kInterruptType>
    static constexpr size_t GetTableSize_()
    {
        if constexpr (kInterruptType == InterruptType::kException) {
            return kNumExceptions;
        } else if constexpr (kInterruptType == InterruptType::kHardwareException) {
            return kNumHardwareExceptions;
        } else if constexpr (kInterruptType == InterruptType::kSoftwareException) {
            return kNumSoftwareExceptions;
        } else {
            R_FAIL_ALWAYS("Invalid type provided");
        }
    }

    template <InterruptType kInterruptType>
    NODISCARD FORCE_INLINE_F bool IsUnmapped_(const u16 lirq)
    {
        ASSERT_LT(lirq, GetTableSize_<kInterruptType>());
        return GetTable_<kInterruptType>()[lirq].hardware_irq == kUnmappedIrq;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    InterruptHandlerEntry<InterruptType::kException> exception_table_[kNumExceptions];
    InterruptHandlerEntry<InterruptType::kHardwareException>
        hardware_exception_table_[kNumHardwareExceptions];
    InterruptHandlerEntry<InterruptType::kSoftwareException>
        software_exception_table_[kNumSoftwareExceptions];
};
}  // namespace intr

#include "logical_interrupt_table.tpp"

#endif  // ALKOS_KERNEL_INCLUDE_INTERRUPTS_LOGICAL_INTERRUPT_TABLE_HPP_
