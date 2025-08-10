#ifndef ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_

#include <extensions/template_lib.hpp>

#include "acpi/acpi.hpp"
#include "hardware/clock_infra.hpp"
#include "hardware/cores.hpp"
#include "hardware/event_clock_infra.hpp"
#include "hardware/interupts.hpp"
#include "modules/helpers.hpp"

namespace internal
{
class HardwareModule : template_lib::StaticSingletonHelper
{
    // -------------------------------------
    // Protected singleton constructor
    // -------------------------------------

    protected:
    HardwareModule() noexcept;

    // ------------------------------
    // Module fields
    // ------------------------------

    DEFINE_MODULE_FIELD(ACPI, ACPIController)
    DEFINE_MODULE_FIELD(hardware, Interrupts)
    DEFINE_MODULE_FIELD(hardware, CoresController)
    DEFINE_MODULE_FIELD(hardware, ClockRegistry)
    DEFINE_MODULE_FIELD(hardware, EventClockRegistry)
};
}  // namespace internal

using HardwareModule = template_lib::StaticSingleton<internal::HardwareModule>;

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_HARDWARE_HPP_
