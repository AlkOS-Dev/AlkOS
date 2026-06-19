// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_INTERRUPTS_INTERUPTS_HPP_
#define KERNEL_SRC_INTERRUPTS_INTERUPTS_HPP_

#include "hal/interrupts.hpp"
#include "interrupts/logical_interrupt_table.hpp"

namespace intr
{

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

#endif  // KERNEL_SRC_INTERRUPTS_INTERUPTS_HPP_
