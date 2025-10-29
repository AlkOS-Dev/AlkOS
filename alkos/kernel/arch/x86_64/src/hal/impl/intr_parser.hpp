#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTR_PARSER_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTR_PARSER_HPP_

#include <hal/api/intr_parser.hpp>
#include <mem/virt/page_fault_data.hpp>

#include "hal/impl/interrupt_params.hpp"

namespace arch
{

Mem::PageFaultData ParsePageFaultData(const ExceptionData &ed);

}

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_IMPL_INTR_PARSER_HPP_
