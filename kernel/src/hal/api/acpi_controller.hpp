#ifndef ALKOS_KERNEL_INCLUDE_HAL_API_ACPI_CONTROLLER_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_API_ACPI_CONTROLLER_HPP_

namespace arch
{

class AcpiController;

struct AcpiAPI {
    void ParseTables();
};

}  // namespace arch

#endif  // ALKOS_KERNEL_INCLUDE_HAL_API_ACPI_CONTROLLER_HPP_
