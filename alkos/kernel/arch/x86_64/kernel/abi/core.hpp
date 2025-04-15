#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CORE_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CORE_HPP_

#include "core.hpp"

namespace arch
{
class Core : public CoreABI
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    Core() = default;

    ~Core() = default;

    // ------------------------------
    // ABI implementation
    // ------------------------------

    void EnableCore()
    {
        // TODO:
    }

    // ------------------------------
    // Class fields
    // ------------------------------
};
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_CORE_HPP_
