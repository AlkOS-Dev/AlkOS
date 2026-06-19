// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_ARCH_X86_64_SRC_HAL_IMPL_ACPI_CONTROLLER_HPP_
#define KERNEL_ARCH_X86_64_SRC_HAL_IMPL_ACPI_CONTROLLER_HPP_

#include <hal/api/acpi_controller.hpp>

namespace arch
{
class AcpiController : public AcpiAPI
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    // ------------------------------
    // ABI implementation
    // ------------------------------

    void ParseTables();

    // ------------------------------
    // Class methods
    // ------------------------------

    private:
    void ParseMadt_();
    void ParseHpet_();

    // ------------------------------
    // Class fields
    // ------------------------------
};
}  // namespace arch

#endif  // KERNEL_ARCH_X86_64_SRC_HAL_IMPL_ACPI_CONTROLLER_HPP_
