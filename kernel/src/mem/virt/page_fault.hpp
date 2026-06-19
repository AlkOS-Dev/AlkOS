// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_MEM_VIRT_PAGE_FAULT_HPP_
#define KERNEL_SRC_MEM_VIRT_PAGE_FAULT_HPP_

#include "hal/interrupt_params.hpp"
#include "interrupts/interrupt_types.hpp"

namespace Mem
{

Sched::Thread *PageFaultHandler(intr::LitExcEntry &entry, hal::ExceptionData *data);

}  // namespace Mem

#endif  // KERNEL_SRC_MEM_VIRT_PAGE_FAULT_HPP_
