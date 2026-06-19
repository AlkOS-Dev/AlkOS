// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_API_ACPI_CONTROLLER_HPP_
#define KERNEL_SRC_HAL_API_ACPI_CONTROLLER_HPP_

namespace arch
{

class AcpiController;

struct AcpiAPI {
    void ParseTables();
};

}  // namespace arch

#endif  // KERNEL_SRC_HAL_API_ACPI_CONTROLLER_HPP_
