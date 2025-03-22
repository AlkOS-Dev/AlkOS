#ifndef ALKOS_KERNEL_INCLUDE_ACPI_INTERRUPT_HPP_
#define ALKOS_KERNEL_INCLUDE_ACPI_INTERRUPT_HPP_

#include <extensions/defines.hpp>
#include <extensions/types.hpp>

namespace ACPI
{

//////////////////////////////
//          Enums           //
//////////////////////////////

//////////////////////////////
//         Structs          //
//////////////////////////////

/**
 * @struct InterruptRoute
 * @brief Represents an interrupt routing entry.
 */
STRUCT PACK InterruptRoute
{
    u8 bus;
    u8 device;
    u8 pin;     // INTA, INTB, INTC, or INTD (0-3)
    u8 gsi;     // Global System Interrupt
    u16 flags;  // Polarity, trigger mode, etc.
}
InterruptRoute;

//////////////////////////////
//        Functions         //
//////////////////////////////

}  // namespace ACPI

#endif  // ALKOS_KERNEL_INCLUDE_ACPI_INTERRUPT_HPP_
