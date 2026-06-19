// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#include "hal/impl/intr_parser.hpp"
#include "hal/impl/interrupt_params.hpp"

#include "cpu/control_registers.hpp"

#include <bit.hpp>
#include <mem/types.hpp>

namespace arch
{

Mem::PageFaultData ParsePageFaultData(const ExceptionData &ed)
{
    Mem::PageFaultData data{};

    cpu::Cr2 cr2      = cpu::GetCR<cpu::Cr2>();
    data.faulting_ptr = reinterpret_cast<Mem::VPtr<void>>(cr2.PageFaultLinearAddress);

    const PageFaultErrorCode ec = *reinterpret_cast<const PageFaultErrorCode *>(&ed.error_code);

    data.error.present           = ec.present;
    data.error.write             = ec.write;
    data.error.user              = ec.user;
    data.error.reserved_bits     = ec.reserved;
    data.error.instruction_fetch = ec.instruction_fetch;

    return data;
}

}  // namespace arch
