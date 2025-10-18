#ifndef ALKOS_KERNEL_INCLUDE_INTERRUPTS_INTERUPTS_HPP_
#define ALKOS_KERNEL_INCLUDE_INTERRUPTS_INTERUPTS_HPP_

#include <hal/interrupts.hpp>
#include "interrupts/logical_interrupt_table.hpp"

namespace intr
{
/* Simplify programming interface */
using LitType = LogicalInterruptTable<
    hal::kNumSoftwareInterrupts, hal::kNumHardwareInterrupts, hal::kNumSoftwareInterrupts>;

template <InterruptType kInterruptType>
using HandlerData = LitType::HandlerData<kInterruptType>;

template <InterruptType kInterruptType>
using HandlerType = LitType::HandlerType<kInterruptType>;

using InterruptDriver = LitType::InterruptDriver;

class Interrupts final : public arch::Interrupts
{
    // ------------------------------
    // Class creation
    // ------------------------------

    public:
    Interrupts() noexcept = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    FORCE_INLINE_F LitType &GetLit() { return lit_; }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    LitType lit_{};
};
}  // namespace intr

#endif  // ALKOS_KERNEL_INCLUDE_INTERRUPTS_INTERUPTS_HPP_
