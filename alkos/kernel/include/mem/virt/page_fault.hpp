#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_PAGE_FAULT_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_PAGE_FAULT_HPP_

#include "hal/interrupt_params.hpp"
#include "interrupts/interrupt_types.hpp"

namespace Mem
{

void PageFaultHandler(intr::LitExcEntry &entry, hal::ExceptionData *data);

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VIRT_PAGE_FAULT_HPP_
