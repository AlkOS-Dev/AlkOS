#ifndef ALKOS_KERNEL_ABI_INTERRUPTS_HPP_
#define ALKOS_KERNEL_ABI_INTERRUPTS_HPP_

namespace arch
{
/* Should be defined by architecture, all interrupts handling and state should be stored here */
class Interrupts;

/* Abi which Interrupts class should follow */
struct InterruptsABI {
};

}  // namespace arch

/* Load architecture definition of component */
#include <abi/interrupts.hpp>

#endif  // ALKOS_KERNEL_ABI_INTERRUPTS_HPP_
