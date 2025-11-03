#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_ACPI_CONTROLLER_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_ACPI_CONTROLLER_HPP_

#include <hal/api/acpi_controller.hpp>

namespace arch
{
class AcpiController : public AcpiAPI
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
    void ParseHpet_();

    // ------------------------------
    // Class fields
    // ------------------------------
};
}  // namespace arch

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_ABI_ACPI_CONTROLLER_HPP_
