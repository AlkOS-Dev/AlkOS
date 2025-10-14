#ifndef ALKOS_KERNEL_INCLUDE_HAL_API_INTERRUPTS_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_API_INTERRUPTS_HPP_

#include "stddef.h"

namespace arch
{
class Interrupts;
class IsrArguments;
using IsrHandler = void (*)(const IsrArguments& args);

struct InterruptsAPI {
    /* Should perform full initialisation of interrupt system */
    void Init();
    void InstallIsrHandler(size_t isr_id, IsrHandler handler);
};

}  // namespace arch

#endif  // ALKOS_KERNEL_INCLUDE_HAL_API_INTERRUPTS_HPP_
