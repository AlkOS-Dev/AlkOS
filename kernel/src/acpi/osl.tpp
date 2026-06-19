// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef ALKOS_KERNEL_SRC_ACPI_TPP_
#define ALKOS_KERNEL_SRC_ACPI_TPP_

#include <uacpi/status.h>
#include <uacpi/types.h>
#include <concepts_ext.hpp>

// TODO SHOULD BE THROUGH HAL
#include <include/io.hpp>
#include <include/pci.hpp>
#include <mem/types.hpp>

template <concepts_ext::IoT T>
uacpi_status uacpi_kernel_pci_read(uacpi_handle device, uacpi_size offset, T *value)
{
    using namespace Mem;
    const VPtr<uacpi_pci_address> addr = PhysToVirt(static_cast<PPtr<uacpi_pci_address>>(device));
    *value = pci::read<T>(addr->bus, addr->device, addr->function, offset);
    return UACPI_STATUS_OK;
}

template <concepts_ext::IoT T>
uacpi_status uacpi_kernel_pci_write(uacpi_handle device, uacpi_size offset, T value)
{
    using namespace Mem;
    const VPtr<uacpi_pci_address> addr = PhysToVirt(static_cast<PPtr<uacpi_pci_address>>(device));
    pci::write<T>(addr->bus, addr->device, addr->function, offset, value);
    return UACPI_STATUS_OK;
}

template <concepts_ext::IoT T>
uacpi_status uacpi_kernel_raw_io_write(uacpi_io_addr address, T in_value)
{
    io::out(address, in_value);
    return UACPI_STATUS_OK;
}

template <concepts_ext::IoT T>
uacpi_status uacpi_kernel_raw_io_read(uacpi_io_addr address, T *out_value)
{
    *out_value = io::in<T>(address);
    return UACPI_STATUS_OK;
}

#endif  // ALKOS_KERNEL_SRC_ACPI_TPP_
