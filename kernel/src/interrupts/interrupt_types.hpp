#ifndef KERNEL_SRC_INTERRUPTS_INTERRUPT_TYPES_HPP_
#define KERNEL_SRC_INTERRUPTS_INTERRUPT_TYPES_HPP_

#include <template_lib.hpp>
#include "hal/interrupt_params.hpp"

namespace intr
{
enum class InterruptType : uint8_t {
    kException         = 0,
    kHardwareInterrupt = 1,
    kSoftwareInterrupt = 2,
};

template <InterruptType kInterruptType>
struct InterruptHandlerEntry {
    /* Interrupt handler */
    using InterruptHandler = void (*)(InterruptHandlerEntry &entry);
    using InterruptHandlerException =
        void (*)(InterruptHandlerEntry &entry, hal::ExceptionData *data);
    using HandlerType = std::conditional_t<
        kInterruptType == InterruptType::kException, InterruptHandlerException, InterruptHandler>;

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
        kInterruptType == InterruptType::kHardwareInterrupt, InterruptDriver *>
        driver{};
};

template <InterruptType kInterruptType>
using HandlerData = typename InterruptHandlerEntry<kInterruptType>::HandlerData;

template <InterruptType kInterruptType>
using HandlerType = typename InterruptHandlerEntry<kInterruptType>::HandlerType;

using InterruptDriver = InterruptHandlerEntry<InterruptType::kHardwareInterrupt>::InterruptDriver;

using ExcHandler = HandlerData<InterruptType::kException>;
using HwHandler  = HandlerData<InterruptType::kHardwareInterrupt>;
using SwHandler  = HandlerData<InterruptType::kSoftwareInterrupt>;

using LitExcEntry = InterruptHandlerEntry<InterruptType::kException>;
using LitHwEntry  = InterruptHandlerEntry<InterruptType::kHardwareInterrupt>;
using LitSwEntry  = InterruptHandlerEntry<InterruptType::kSoftwareInterrupt>;
}  // namespace intr

#endif  // KERNEL_SRC_INTERRUPTS_INTERRUPT_TYPES_HPP_
