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
