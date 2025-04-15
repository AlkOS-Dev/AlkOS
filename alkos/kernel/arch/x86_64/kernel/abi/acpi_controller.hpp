#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_ACPI_CONTROLLER_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_ACPI_CONTROLLER_HPP_

#include "acpi_controller.hpp"

namespace arch
{
class AcpiController : public AcpiABI
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    // ------------------------------
    // ABI implementation
    // ------------------------------

    void ParseTables();

    // ------------------------------
    // Class methods
    // ------------------------------

    private:
    void ParseMadt_();

    // ------------------------------
    // Class fields
    // ------------------------------
};
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_ACPI_CONTROLLER_HPP_
