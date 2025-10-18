#ifndef ALKOS_KERNEL_INCLUDE_INTERRUPTS_INTERUPTS_HPP_
#define ALKOS_KERNEL_INCLUDE_INTERRUPTS_INTERUPTS_HPP_

#include "interrupts/interrupt_types.hpp"

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

#endif  // ALKOS_KERNEL_INCLUDE_INTERRUPTS_INTERUPTS_HPP_
