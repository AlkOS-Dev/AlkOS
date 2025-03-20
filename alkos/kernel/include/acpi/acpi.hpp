#ifndef ALKOS_KERNEL_INCLUDE_ACPI_HPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_HPP_

#include <uacpi/tables.h>

#include <extensions/types.hpp>
namespace ACPI
{

int Init();
uacpi_table GetRSDT();
uacpi_table GetXSDT();
uacpi_table GetFADT();
uacpi_table GetMADT();
uacpi_table GetDSDT();
uacpi_table GetSSDT();
uacpi_table GetSRAT();
bool Shutdown();
bool Reboot();

}  // namespace ACPI

#endif  // ALKOS_KERNEL_INCLUDE_ACPI_HPP_
