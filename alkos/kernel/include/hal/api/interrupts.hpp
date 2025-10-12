#ifndef ALKOS_KERNEL_INCLUDE_HAL_API_INTERRUPTS_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_API_INTERRUPTS_HPP_

namespace arch
{
class Interrupts;

struct InterruptsAPI {
    /* Should perform full initialisation of interrupt system */
    void Init();
};

}  // namespace arch

#endif  // ALKOS_KERNEL_INCLUDE_HAL_API_INTERRUPTS_HPP_
