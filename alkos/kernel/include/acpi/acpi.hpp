#ifndef ALKOS_KERNEL_INCLUDE_ACPI_ACPI_HPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_ACPI_HPP_

#include <uacpi/uacpi.h>
#include <extensions/defines.hpp>

#include "acpi_battery.hpp"
#include "acpi_cpu.hpp"
#include "acpi_device.hpp"
#include "acpi_interrupt.hpp"
#include "acpi_power.hpp"
#include "acpi_tables.hpp"
#include "acpi_thermal.hpp"

namespace ACPI
{

//////////////////////////////
//          Enums           //
//////////////////////////////

//////////////////////////////
//         Structs          //
//////////////////////////////

//////////////////////////////
//        Functions         //
//////////////////////////////

/**
 * @brief Initialize the ACPI subsystem.
 * @return Status code
 */
int Init();

/**
 * @brief Deinitialize the ACPI subsystem.
 */
WRAP_CALL void Deinit() { uacpi_state_reset(); }

}  // namespace ACPI

#include "acpi.tpp"

#endif  // ALKOS_KERNEL_INCLUDE_ACPI_ACPI_HPP_
