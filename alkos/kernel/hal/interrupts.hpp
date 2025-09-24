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
#include <hal/interrupts.hpp>

#endif  // ALKOS_KERNEL_ABI_INTERRUPTS_HPP_
