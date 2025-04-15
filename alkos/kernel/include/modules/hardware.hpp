#ifndef ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_

#include <acpi/acpi.hpp>
#include <extensions/template_lib.hpp>
#include <hardware/interupts.hpp>

namespace internal
{
class HardwareModule : TemplateLib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    HardwareModule() noexcept;

    // ------------------------------
    // Getters
    // ------------------------------

    public:
    FORCE_INLINE_F hardware::Interrupts& GetInterrupts() noexcept { return interrupts_; }
    FORCE_INLINE_F ACPI::ACPIController& GetAcpiController() noexcept { return acpi_controller_; }

    // ------------------------------
    // Module fields
    // ------------------------------

    private:
    ACPI::ACPIController acpi_controller_;
    hardware::Interrupts interrupts_;
};
}  // namespace internal

using HardwareModule = TemplateLib::StaticSingleton<internal::HardwareModule>;

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_
