#ifndef ALKOS_KERNEL_SRC_ACPI_TPP_
#define ALKOS_KERNEL_SRC_ACPI_TPP_

#include <uacpi/status.h>
#include <uacpi/types.h>
#include <extensions/debug.hpp>
#include <extensions/type_traits.hpp>
#include <io.hpp>

template <typename T>
concept ValidUnsigned =
    std::is_unsigned_v<T> && (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4);

template <ValidUnsigned T>
uacpi_status uacpi_kernel_pci_read(uacpi_handle device, uacpi_size offset, T *value)
{
    const uacpi_pci_address addr = *static_cast<uacpi_pci_address *>(device);
    ASSERT(addr.device < 32 && addr.function < 8);

    const auto address = (addr.bus << 16) | (addr.device << 11) | (addr.function << 8) |
                         (offset & 0xFC) | 0x80000000;

    outl(address, 0xCF8);

    offset &= 3;  // Address must be 32-bit aligned
    switch (sizeof(T)) {
        case 1:
            *value = inb(0xCFC + offset);
            break;
        case 2:
            *value = inw(0xCFC + offset);
            break;
        case 4:
            *value = inl(0xCFC + offset);
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }

    return UACPI_STATUS_OK;
}

template <ValidUnsigned T>
uacpi_status uacpi_kernel_pci_write(uacpi_handle device, uacpi_size offset, T value)
{
    const uacpi_pci_address addr = *static_cast<uacpi_pci_address *>(device);
    ASSERT(addr.device < 32 && addr.function < 8);

    const auto address = (addr.bus << 16) | (addr.device << 11) | (addr.function << 8) |
                         (offset & 0xFC) | 0x80000000;

    outl(address, 0xCF8);

    offset &= 3;  // Address must be 32-bit aligned
    switch (sizeof(T)) {
        case 1:
            outb(0xCFC + offset, value);
            break;
        case 2:
            outw(0xCFC + offset, value);
            break;
        case 4:
            outl(0xCFC + offset, value);
            break;
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }

    return UACPI_STATUS_OK;
}

template <ValidUnsigned T>
uacpi_status uacpi_kernel_raw_io_write(uacpi_io_addr address, T in_value)
{
    switch (sizeof(T)) {
        case 1: {
            outb(address, in_value);
            break;
        }
        case 2: {
            outw(address, in_value);
            break;
        }
        case 4: {
            outl(address, in_value);
            break;
        }
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }

    return UACPI_STATUS_OK;
}

template <ValidUnsigned T>
uacpi_status uacpi_kernel_raw_io_read(uacpi_io_addr address, T *out_value)
{
    switch (sizeof(T)) {
        case 1: {
            *out_value = inb(address);
            break;
        }
        case 2: {
            *out_value = inw(address);
            break;
        }
        case 4: {
            *out_value = inl(address);
            break;
        }
        default:
            return UACPI_STATUS_INVALID_ARGUMENT;
    }

    return UACPI_STATUS_OK;
}

#endif  // ALKOS_KERNEL_SRC_ACPI_TPP_
