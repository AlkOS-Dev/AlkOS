#ifndef ALKOS_KERNEL_ABI_ACPI_CONTROLLER_HPP_
#define ALKOS_KERNEL_ABI_ACPI_CONTROLLER_HPP_

#include <extensions/type_traits.hpp>

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
#include <abi/acpi_controller.hpp>
static_assert(
    std::is_base_of_v<arch::AcpiABI, arch::AcpiController>,
    "AcpiController must derive from AcpiABI..."
);

#endif  // ALKOS_KERNEL_ABI_ACPI_CONTROLLER_HPP_
