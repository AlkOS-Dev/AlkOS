#ifndef ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_

#include <extensions/template_lib.hpp>

#include "acpi/acpi.hpp"
#include "hardware/cores.hpp"
#include "hardware/interupts.hpp"

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
    NODISCARD FORCE_INLINE_F hardware::Interrupts &GetInterrupts() noexcept { return interrupts_; }
    NODISCARD FORCE_INLINE_F ACPI::ACPIController &GetAcpiController() noexcept
    {
        return acpi_controller_;
    }
    NODISCARD FORCE_INLINE_F hardware::CoresController &GetCoresController() noexcept
    {
        return cores_;
    }

    // ------------------------------
    // Module fields
    // ------------------------------

    private:
    ACPI::ACPIController acpi_controller_{};
    hardware::Interrupts interrupts_{};
    hardware::CoresController cores_{};
};
}  // namespace internal

using HardwareModule = TemplateLib::StaticSingleton<internal::HardwareModule>;

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_
