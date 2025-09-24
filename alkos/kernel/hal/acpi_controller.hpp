#ifndef ALKOS_KERNEL_ABI_ACPI_CONTROLLER_HPP_
#define ALKOS_KERNEL_ABI_ACPI_CONTROLLER_HPP_

#include <extensions/type_traits.hpp>

namespace arch
{
/* Should be defined by architecture, all Acpi logic correlated to architecture and state should be
 * stored here */
class AcpiController;

/* Abi which AcpiController class should follow */
struct AcpiABI {
    void ParseTables();
};

}  // namespace arch

/* Load architecture definition of component */
#include <hal/acpi_controller.hpp>

#endif  // ALKOS_KERNEL_ABI_ACPI_CONTROLLER_HPP_
