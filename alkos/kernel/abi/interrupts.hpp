#ifndef ALKOS_KERNEL_ABI_INTERRUPTS_HPP_
#define ALKOS_KERNEL_ABI_INTERRUPTS_HPP_

#include <extensions/type_traits.hpp>

namespace arch
{
/* Should be defined by architecture, all interrupts handling and state should be stored here */
class Interrupts;

/* Abi which Interrupts class should follow */
struct InterruptsABI {
    /* Should perform full initialisation of interrupt system */
    void Initialise();
};

}  // namespace arch

/* Load architecture definition of component */
#include <abi/interrupts.hpp>
static_assert(
    std::is_base_of_v<arch::InterruptsABI, arch::Interrupts>,
    "Interrupts must derive from InterruptsABI..."
);

#endif  // ALKOS_KERNEL_ABI_INTERRUPTS_HPP_
