#ifndef ALKOS_KERNEL_SRC_ACPI_TPP_
#define ALKOS_KERNEL_SRC_ACPI_TPP_

#include <uacpi/status.h>
#include <uacpi/types.h>
#include <extensions/concepts_ext.hpp>
#include <io.hpp>
#include <pci.hpp>

template <concepts_ext::IoT T>
uacpi_status uacpi_kernel_pci_read(uacpi_handle device, uacpi_size offset, T *value)
{
    const uacpi_pci_address addr = *static_cast<uacpi_pci_address *>(device);
    *value                       = pci::read<T>(addr.bus, addr.device, addr.function, offset);
    return UACPI_STATUS_OK;
}

template <concepts_ext::IoT T>
uacpi_status uacpi_kernel_pci_write(uacpi_handle device, uacpi_size offset, T value)
{
    const uacpi_pci_address addr = *static_cast<uacpi_pci_address *>(device);
    pci::write<T>(addr.bus, addr.device, addr.function, offset, value);
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
