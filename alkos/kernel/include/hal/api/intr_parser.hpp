#ifndef ALKOS_KERNEL_INCLUDE_HAL_API_INTR_PARSER_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_API_INTR_PARSER_HPP_

/*
 * This file contains functions that parse/translate data given
 * during an interrupt into an abstract hardware independent object
 *
 * Used in generic handlers e.g. PageFault
 */

#include "hal/api/interrupts_params.hpp"
#include "mem/types.hpp"
#include "mem/virt/page_fault_data.hpp"

namespace arch
{

/**
 * @brief Extracts page fault data from architecture-specific exception information.
 *
 * This function serves as a getter that translates raw, architecture-specific
 * exception data, including CPU state and registers, into a structured,
 * architecture-independent object.
 */
Mem::PageFaultData ParsePageFaultData(const ExceptionData &ed);

}  // namespace arch

#endif  // ALKOS_KERNEL_INCLUDE_HAL_API_INTR_PARSER_HPP_
