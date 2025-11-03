#ifndef ALKOS_KERNEL_INCLUDE_INTERRUPTS_INTERRUPT_TYPES_HPP_
#define ALKOS_KERNEL_INCLUDE_INTERRUPTS_INTERRUPT_TYPES_HPP_

#include "hal/interrupt_params.hpp"
#include "interrupts/logical_interrupt_table.tpp"

namespace intr
{
/* Simplify programming interface */
using LitType = LogicalInterruptTable<
    hal::kNumSoftwareInterrupts, hal::kNumHardwareInterrupts, hal::kNumSoftwareInterrupts>;

template <InterruptType kInterruptType>
using HandlerData = LitType::HandlerData<kInterruptType>;
using ExcHandler  = HandlerData<InterruptType::kException>;
using HwHandler   = HandlerData<InterruptType::kHardwareInterrupt>;
using SwHandler   = HandlerData<InterruptType::kSoftwareInterrupt>;

template <InterruptType kInterruptType>
using HandlerType = LitType::HandlerType<kInterruptType>;

using InterruptDriver = LitType::InterruptDriver;

template <InterruptType kInterruptType>
using LitEntry    = LitType::InterruptHandlerEntry<kInterruptType>;
using LitExcEntry = LitEntry<InterruptType::kException>;
using LitHwEntry  = LitEntry<InterruptType::kHardwareInterrupt>;
using LitSwEntry  = LitEntry<InterruptType::kSoftwareInterrupt>;
}  // namespace intr

#endif  // ALKOS_KERNEL_INCLUDE_INTERRUPTS_INTERRUPT_TYPES_HPP_
