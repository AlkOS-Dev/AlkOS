#ifndef ALKOS_KERNEL_ABI_CORE_HPP_
#define ALKOS_KERNEL_ABI_CORE_HPP_

namespace arch
{
/* Should be defined by architecture, all CPU cores handling and state should be stored here */
class Core;

/* Abi which Core class should follow */
struct CoreABI {
    /* Should perform full initialisation of single core */
    void EnableCore();
};

}  // namespace arch

/* Load architecture definition of component */
#include <abi/core.hpp>

#endif  // ALKOS_KERNEL_ABI_CORE_HPP_
