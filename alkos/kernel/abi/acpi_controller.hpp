#ifndef ALKOS_KERNEL_ABI_ACPI_HPP_
#define ALKOS_KERNEL_ABI_ACPI_HPP_

namespace arch
{
/* Should be defined by architecture, all Acpi logic correlated to arhcitecture and state should be
 * stored here */
class AcpiController;

/* Abi which AcpiController class should follow */
struct AcpiABI {
    void ParseTables();
};

}  // namespace arch

/* Load architecture definition of component */
#include <abi/acpi.hpp>

#endif  // ALKOS_KERNEL_ABI_ACPI_HPP_
